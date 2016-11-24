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
	std::ostream& operator<< (std::ostream& out, ion::Matrix<T>& mat)
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

	//explicit instantiations
	//double
	template std::ostream& operator<< (std::ostream& out, ion::Matrix<double>& mat);
	//uchar
	template std::ostream& operator<< (std::ostream& out, ion::Matrix<uint8_t>& mat);
	template std::ostream& operator<< (std::ostream& out, ion::Matrix<uint32_t>& mat);
} //namespace ion
