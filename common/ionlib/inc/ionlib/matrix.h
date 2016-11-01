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
#ifndef ION_MATRIX_H_
#define ION_MATRIX_H_
#include <stdint.h>
#include "ionlib/log.h"
#include <string>
#include <sstream>
#include <iostream>
#define MAT_INDEX(mat,x,y,z) ((x)*(mat).pages_*(mat).cols_+(y)*(mat).pages_+(z))

namespace ion
{
	template <class T>
	class Matrix
	{
	public:
		Matrix(uint32_t rows, uint32_t cols=1, uint32_t pages=1)
		{
			//consider checking here if rows*cols*pages fits in a 64 bit integer
			allocated_cells_ = (uint64_t)rows * (uint64_t)cols * (uint64_t)pages;
			//allocate the matrix
			data_ = new (std::nothrow) T[allocated_cells_];
			LOGASSERT(data_ != NULL);
			rows_ = rows;
			cols_ = cols;
			pages_ = pages;
			continuous_ = true;
		}
		void Resize(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1)
		{
			uint64_t new_size = (uint64_t)rows * (uint64_t)cols * (uint64_t)pages;
			if (new_size > allocated_cells_)
			{
				LOGWARN("The requested matrix size %ull is greater than the allocated size %ull. The matrix will be re-allocated but this can lead to severe performance penalties");
				delete data_;
				allocated_cells_ = new_size;
				data_ = new (std::nothrow) T[allocated_cells_];
				LOGASSERT(data != NULL);
			}
			rows_ = rows;
			cols_ = cols;
			pages_ = pages;
		}
		void Swap(Matrix<T> rhs)
		{
			//make a shallow copy and use that to swap
			Matrix temp;
			temp = rhs;
			rhs = *this;
			*this = temp;
		}
		template <class OtherType>
		void Cast(Matrix<OtherType> rhs)
		{
			LOGASSERT(rhs->rows_ == rows_);
			LOGASSERT(rhs->cols_ == cols_);
			LOGASSERT(rhs->pages_ == pages_);

			for (uint32_t row_index = 0; row_index < rows; ++row_index)
			{
				for (uint32_t col_index = 0; col_index < cols; ++col_index)
				{
					for (uint32_t page_index = 0; page_index < pages; ++page_index)
					{
						data_[MAT_INDEX(*this, row_index, col_index, page_index)] = (T)rhs->data_[MAT_INDEX(rhs, row_index, col_index, page_index)];
					}
				}
			}
		}
		void Reshape(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1)
		{
			//this function maintains the data, whereas Resize doesn't have to. For example if *this is a row vector and you resize it to a column vector it will work, whereas Resize may not
			uint64_t new_size = (uint64_t)rows * (uint64_t)cols * (uint64_t)pages;
			//note that old_size is not necessarily equal to allocated_cells_ because the matrix could be a region of interest
			uint64_t old_size = (uint64_t)rows_ * (uint64_t)cols_* (uint64_t)pages_;
			LOGASSERT(new_size == old_size);
			//Since reshape only accepts new shapes of the same total size, there is no chance that data_ has to be reallocated
			//But for now we will allocate new space to do the reshape. Note that this isn't necessary: a shceme could be developed to reshape without an extra allocation
			Matrix<T> old_mat(rows, cols, pages);
			Swap(old_mat);

			for (uint32_t row_index = 0; row_index < rows; ++row_index)
			{
				for (uint32_t col_index = 0; col_index < cols; ++col_index)
				{
					for (uint32_t page_index = 0; page_index < pages; ++page_index)
					{
						//compute the indices in the source matrix
						//Note that we could just iterate through the elements as long as the matrix is not a region of interest
						uint64_t src_offset = MAT_INDEX(*this, row_index, col_index, page_index);
						uint32_t src_row = (uint32_t)(src_offset / (old_mat.cols_*old_mat.pages_));
						uint32_t src_col = (uint32_t)((src_offset - (src_row*old_mat.cols_*old_mat.pages_)) / (old_mat.pages_));
						uint32_t src_page = (uint32_t)((src_offset % (src_row * old_mat.cols_ * old_mat.pages_ + src_col * old_mat.pages_)));
						data_[MAT_INDEX(*this, page_index, col_index, row_index)] = old_mat.data_[MAT_INDEX(old_mat, src_row, src_col, src_page)];
					}
				}
			}
			//old_mat will automatically be deleted by the destructor
		}
		void Set(T val, uint32_t x, uint32_t y = 0, uint32_t z = 0)
		{
			data_[MAT_INDEX(*this, x, y, z)] = val;
		}
		T At(uint32_t x, uint32_t y=0, uint32_t z=0)
		{
			return data_[MAT_INDEX(*this, x, y, z)];
		}
		void Zero()
		{
			//check if the mat is continuous. If so it can simply be memset
			if (continuous_)
			{
				memset(data_, 0, allocated_cells_);
			} else
			{
				//do it the hard way
				for (uint32_t row_index = 0; row_index < rows; ++row_index)
				{
					for (uint32_t col_index = 0; col_index < cols; ++col_index)
					{
						for (uint32_t page_index = 0; page_index < pages; ++page_index)
						{
							data_[MAT_INDEX(*this, row_index, col_index, page_index)] = static_cast<T>(0);
						}
					}
				}
			}
		}
		void Eye()
		{
			//this can only be run on both square matrices
			LOGASSERT(rows_ == cols_ && (pages_ == 1));
			Zero();
			for (uint32_t row_index = 0; row_index < rows; ++row_index)
			{
				data_[MAT_INDEX(*this, row_index, col_index, 1)] = static_cast < T>1;
			}
		}
		void Print(std::ostream& stream)
		{
			stream << "(" << rows_ << "," << cols_ << "," << pages_ << ")" << std::endl;
			stream << "[";
			for (uint32_t row_index = 0; row_index < rows_; ++row_index)
			{
				if (cols_ > 1)
				{
					stream << "[";
				}
				for (uint32_t col_index = 0; col_index < cols_; ++col_index)
				{
					if (pages_ > 1)
					{
						stream << "[";
					}
					for (uint32_t page_index = 0; page_index < pages_; ++page_index)
					{
						stream << data_[MAT_INDEX(*this, row_index, col_index, page_index)];
						if (page_index != pages_ - 1)
						{
							stream << ",";
						}
					}
					//delete the last comma
					//stream.seekp(-1, stream.cur);
					if (pages_ > 1)
					{
						stream << "]";
					}
				}
				if (cols_ > 1)
				{
					stream << "]";
					if (row_index != rows_ - 1)
					{
						stream << std::endl;
					}
				}
			}
			stream << "]";
		}
		std::string Sprintf()
		{
			std::stringstream buf;
			Print(buf);
			return buf.str();
		}
		uint32_t rows()
		{
			return rows_;
		}
		uint32_t cols()
		{
			return cols_;
		}
		uint32_t pages()
		{
			return pages_;
		}
	private:
		Matrix() { } //default construction is only allowed by the library
		uint64_t allocated_cells_;
		uint32_t rows_;
		uint32_t cols_;
		uint32_t pages_;
		bool     continuous_;
		T* data_;
	};
	template <class T>
	std::ostream& operator<< (std::ostream& out, ion::Matrix<T>& mat)
	{
		mat.Print(out);
		return out;
	}
};
#endif //ION_MATRIX_H_