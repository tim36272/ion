/*
This file is part of Ionlib.  Copyright (C) 2016  Tim Sweet

Ionlib is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ionlib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ionlib.If not, see <http://www.gnu.org/licenses/>.
*/
#include "ionlib/matrix.h"
#include "ionlib/math.h"
namespace ion
{
	template <class T>
	void ion::Matrix<T>::PrintAscii(std::ostream& stream) const
	{
		//this function is ROI-safe
		stream << "(" << rows_ << "," << cols_ << "," << pages_ << ")" << std::endl;
		stream << "[";
		bool printed_character_in_this_block = false;
		for (uint32_t page_index = 0; page_index < pages_; ++page_index)
		{
			if (pages_ > 1)
			{
				stream << "[";
			}
			for (uint32_t row_index = 0; row_index < rows_; ++row_index)
			{
				if (cols_ > 1)
				{
					stream << "[";
				}
				for (uint32_t col_index = 0; col_index < cols_; ++col_index)
				{
					//print a comma if there was already a number on this row
					if (printed_character_in_this_block)
					{
						stream << ",";
					}
					stream << At(row_index, col_index, page_index);
					printed_character_in_this_block = true;
				}
				if (cols_ > 1)
				{
					stream << "]";
					printed_character_in_this_block = false;
				}
				if (row_index != rows_ - 1)
				{
					stream << std::endl;
					printed_character_in_this_block = false;
				}
			}
			if (pages_ > 1)
			{
				stream << "]";
				printed_character_in_this_block = false;
			}
			if (page_index != pages_ - 1)
			{
				stream << std::endl;
				printed_character_in_this_block = false;
			}
		}
		stream << "]" << std::endl;
	}
	template <class T>
	void ion::Matrix<T>::DumpBinary(std::ostream& stream) const
	{
		LOGASSERT(contiguous_);
		//Note that row_*cols_*pages_ may or may not be == allocated_cells due to reallocation or an ROI starting at the origin
		//Note that *data here will never fail (even if data == NULL) becaue sizeof(*data_) is actually going to be looked up at compile time
		//The MAT_INDEX is added in case roi_row_origin_ != 0
		//write dimenstions
		stream.write((const char*)&rows_, sizeof(uint32_t));
		stream.write((const char*)&cols_, sizeof(uint32_t));
		stream.write((const char*)&pages_, sizeof(uint32_t));
		size_t bytes_to_write = rows_ * cols_ * pages_ * sizeof(*data_);
		stream.write((const char*)data_ + MAT_INDEX(*this, 0, 0, 0), bytes_to_write);
	}
	template <class T>
	std::string ion::Matrix<T>::Sprintf() const
	{
		std::stringstream buf;
		PrintAscii(buf);
		return buf.str();
	}
	template <class T>
	std::ostream& operator<< (std::ostream& out, const ion::Matrix<T>& mat)
	{
		if (mat.GetPrintFmt() == mat.FMT_ASCII)
		{
			mat.PrintAscii(out);
		} else //if (mat.GetPrintFmt() == ion::Matrix::FRM_BIN)
		{
			mat.DumpBinary(out);
		}
		return out;
	}
	template <class T>
	void ion::Matrix<T>::SetPrintFmt(PrintFmt format)
	{
		format_ = format;
	}
	template <class T>
	typename ion::Matrix<T>::PrintFmt ion::Matrix<T>::GetPrintFmt() const
	{
		return format_;
	}
	template <class T>
	void ion::Matrix<T>::Fread(FILE* fin, size_t num_elements)
	{
		//this isn't strictly required but is easier so I'll enforce continuous until I need otherwise
		LOGASSERT(contiguous_);
		fread_s(data_ + MAT_INDEX(*this, 0, 0, 0), NumCells() * sizeof(T), sizeof(T), num_elements, fin);
	}
	template <class T>
	bool ion::Matrix<T>::SendToSocket(ion::TcpSocket& sock) const {
		LOGASSERT(!is_roi_);
		LOGASSERT(allocated_cells_ < (uint32_t)0xFFFFFFFF);
		//send header
		sock.Send((const char*)this, sizeof(ion::Matrix<T>));
		//send the data
		sock.Send((const char*)data_, uint32_t(sizeof(T) * allocated_cells_));
		return true;
	}
	template <class T>
	bool ion::Matrix<T>::SendToSocketNoData(ion::TcpSocket& sock) const
	{
		LOGASSERT(!is_roi_);
		LOGASSERT(allocated_cells_ < (uint32_t)0xFFFFFFFF);
		//send header
		sock.Send((const char*)this, sizeof(ion::Matrix<T>));
		return true;
	}
	template <class T>
	ion::Matrix<T>::Matrix(ion::TcpSocket& sock, bool receive_data)
	{
		//construct from a network matrix
		//get the header
		ion::Matrix<T> received_matrix(0, 0, 0);
		size_t bytes_read;
		ion::Error result;
		result = sock.Recv((char*)&received_matrix, sizeof(ion::Matrix<T>), NULL, NULL, &bytes_read);
		if (result.success())
		{
			//call my constructor to allocate myself
			Construct(received_matrix.rows_, received_matrix.cols_, received_matrix.pages_, NULL);
			LOGASSERT(allocated_cells_ < (uint32_t)0xFFFFFFFF);
			if (receive_data)
			{
				//get data
				result = sock.Recv((char*)data_, (uint32_t)(allocated_cells_ * sizeof(T)), NULL, NULL, &bytes_read);
				if (!result.success())
				{
					//something went wrong
					LOGFATAL("Failed to receive matrix data in constructor");
					return;
				} else
				{
					return;
				}
			}
			return;
		} else
		{
			LOGFATAL("Failed to receive matrix header in constructor");
			return;
		}
	}
	template <class T>
	bool ion::Matrix<T>::RecvFromSocket(ion::TcpSocket& sock)
	{
		//get the header
		ion::Matrix<T> received_matrix(0, 0, 0);
		size_t bytes_read;
		ion::Error result;
		result = sock.Recv((char*)&received_matrix, sizeof(ion::Matrix<T>), NULL, NULL, &bytes_read);
		if (result.success())
		{
			//verify the matrix is the same size as me
			LOGASSERT(allocated_cells_ == received_matrix.allocated_cells_);
			LOGASSERT(allocated_cells_ < (uint32_t)0xFFFFFFFF);
			LOGASSERT(!received_matrix.is_roi_);
			//receive the data
			result = sock.Recv((char*)data_, (uint32_t)(allocated_cells_ * sizeof(T)), NULL, NULL, &bytes_read);
			if (!result.success())
			{
				//something went wrong
				LOGWARN("Failed to receive matrix data");
				return false;
			} else
			{
				return true;
			}
		} else
		{
			LOGWARN("Failed to receive matrix header");
			return false;
		}
	}
	template <class T>
	void ion::Matrix<T>::Memcpy(T* dest) const
	{
		LOGASSERT(IsContiguous());
		memcpy(dest, data_, rows_*cols_*pages_);
	}
} //namespace ion
