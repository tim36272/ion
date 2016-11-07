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
#define MAT_INDEX(mat,x,y,z) ( ((x)+(mat).roi_row_origin_) * (mat).allocated_pages_ * (mat).allocated_cols_ +\
							  ( (y)+(mat).roi_col_origin_) * (mat).allocated_pages_ +\
								(z)+(mat).roi_page_origin_)

/*
This is how the memory is laid out: if data_ == [1,2,3,4,5,6,7,8,9,A,B,C] then
Page 0:
1 3 5
2 4 6

Page 1:
7 9 B
8 A C

An ROI on rows [0,1] cols[1,2] and page [1,1]:
9 B
A C

*/

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
			allocated_rows_ = rows;
			allocated_cols_ = cols;
			allocated_pages_ = pages;
			roi_row_origin_ = 0;
			roi_col_origin_ = 0;
			roi_page_origin_ = 0;
			continuous_ = true;
			is_roi_ = false;
			format_ = FMT_ASCII;
		}
		void Resize(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1)
		{
			//it doesn't make sense to resize an ROI (just get a new one)
			LOGASSERT(!is_roi_);
			uint64_t new_size = (uint64_t)rows * (uint64_t)cols * (uint64_t)pages;
			if (new_size > allocated_cells_)
			{
				LOGWARN("The requested matrix size %ull is greater than the allocated size %ull. The matrix will be re-allocated but this can lead to severe performance penalties");
				delete data_;
				allocated_cells_ = new_size;
				data_ = new (std::nothrow) T[allocated_cells_];
				LOGASSERT(data_ != NULL);
			}
			rows_ = rows;
			cols_ = cols;
			pages_ = pages;
		}
		void DeepCopyTo(Matrix& rhs)
		{
			LOGASSERT(rows_ == rhs.rows_);
			LOGASSERT(cols_ == rhs.cols_);
			LOGASSERT(pages_ == rhs.pages_);
			//this function is ROI-safe
			//if the input is an ROI, the result will be deep coppied into the parent's data (i.e. it is still an ROI)
			if (continuous_ && rhs.continuous_)
			{
				//Note it is NOT necessary that rows_*cols_*pages_ == allocated_cells_
				//The MAT_INDEX is added in case roi_row_origin_ != 0
				memcpy(rhs.data_ + MAT_INDEX(rhs,0,0,0), data_ + MAT_INDEX(*this,0,0,0), rows_*cols_*pages_);
				return;
			}
			else
			{
				//do it the hard way. We still might be able to optimize part of it, though
				//Notice that, even though it would work, I don't allow it to do this if pages_==1 because it would be more efficient to just assign everything
				if (pages_ != 1 && ((pages_ == allocated_pages_) && (rhs.pages_ == rhs.allocated_pages_)))
				{
					//this means we can copy each *column* via memcpy
					//it is implicit that we can't copy each row otherwise continuous_ would have been set
					for (uint32_t row_index = 0; row_index < rows_; ++row_index)
					{
						for (uint32_t col_index = 0; col_index < cols_; ++col_index)
						{
							//compute where this column starts at
							size_t col_start = MAT_INDEX(*this, row_index, col_index, 0);
							size_t rhs_col_start = MAT_INDEX(rhs, row_index, col_index, 0);
							memcpy(rhs.data_ + rhs_col_start, data_ + col_start, pages_ * sizeof(*data_));
						}
					}
				} else
				{
					//do it the really hard way
					for (uint32_t row_index = 0; row_index < rows_; ++row_index)
					{
						for (uint32_t col_index = 0; col_index < cols_; ++col_index)
						{
							for (uint32_t page_index = 0; page_index < pages_; ++page_index)
							{
								rhs.data_[MAT_INDEX(rhs, row_index, col_index, page_index)] = data_[MAT_INDEX(*this, row_index, col_index, page_index)];
							}
						}
					}
				}
			}
		}
		void Swap(Matrix& rhs)
		{
			//this operation on ROIs might make sense but I'm not sure, so disallow swapping ROIs until I have a use case for it
			LOGASSERT(!is_roi_);
			LOGASSERT(!rhs.is_roi_);
			//make a shallow copy and use that to swap
			Matrix temp;
			temp = rhs;
			rhs = *this;
			*this = temp;
		}
		template <class OtherType>
		void Cast(Matrix<OtherType> rhs)
		{
			//this function is ROI-safe
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
		void Reshape(uint32_t new_rows, uint32_t new_cols = 1, uint32_t new_pages = 1)
		{
			//you cannot reshape an ROI since it doesn't own the data. Deep copy it first.
			LOGASSERT(!is_roi_);
			//this function maintains the data, whereas Resize doesn't have to. For example if *this is a row vector and you resize it to a column vector it will work, whereas Resize may not
			uint64_t new_size = (uint64_t)new_rows * (uint64_t)new_cols * (uint64_t)new_pages;
			//note that old_size is not necessarily equal to allocated_cells_ because the matrix could be a 3x3 allocated in 4x4 cells, for example
			uint64_t old_size = (uint64_t)rows_ * (uint64_t)cols_* (uint64_t)pages_;
			LOGASSERT(new_size == old_size);
			//Since reshape only accepts new shapes of the same total size, there is no chance that data_ has to be reallocated
			//But for now we will allocate new space to do the reshape. Note that this isn't necessary: a scheme could be developed to reshape without an extra allocation
			Matrix<T> old_mat(new_rows, new_cols, new_pages);
			Swap(old_mat);

			for (uint32_t row_index = 0; row_index < new_rows; ++row_index)
			{
				for (uint32_t col_index = 0; col_index < new_cols; ++col_index)
				{
					for (uint32_t page_index = 0; page_index < new_pages; ++page_index)
					{
						//compute the indices in the source matrix
						//Note that we could just iterate through the elements as long as the matrix is not a region of interest
						uint64_t src_offset = MAT_INDEX(*this, row_index, col_index, page_index);
						uint32_t src_row = (uint32_t)(src_offset % old_mat.rows_);
						uint32_t src_col = (uint32_t)((src_offset/old_mat.rows_)%old_mat.cols_);
						uint32_t src_page = (uint32_t)((src_offset / (old_mat.rows_*old_mat.cols_)) % old_mat.pages_);
						data_[MAT_INDEX(*this, row_index, col_index, page_index)] = old_mat.data_[MAT_INDEX(old_mat, src_row, src_col, src_page)];
					}
				}
			}
			//old_mat will automatically be deleted by the destructor
		}
		void Set(T val, uint32_t x, uint32_t y = 0, uint32_t z = 0)
		{
			//this function is ROI-safe
			data_[MAT_INDEX(*this, x, y, z)] = val;
		}
		void SetAll(T val)
		{
			//this function is ROI-safe
			for (uint32_t row_index = 0; row_index < rows_; ++row_index)
			{
				for (uint32_t col_index = 0; col_index < cols_; ++col_index)
				{
					for (uint32_t page_index = 0; page_index < pages_; ++page_index)
					{
						data_[MAT_INDEX(*this, row_index, col_index, page_index)] = val;
					}
				}
			}
		}
		T At(uint32_t x, uint32_t y=0, uint32_t z=0)
		{
			//this function is ROI-safe
			return data_[MAT_INDEX(*this, x, y, z)];
		}
		void Zero()
		{
			//this function is ROI-safe
			//check if the mat is continuous. If so it can simply be memset
			if (continuous_)
			{
				//note that you don't want to memset allocated_cells_ because the data could be continuous but not take up the whole allocated space
				//The MAT_INDEX is added in case roi_row_origin_ != 0
				memset(data_ + MAT_INDEX(*this,0,0,0), 0, rows_ * cols_ * pages_ * sizeof(*data_));
			} else
			{
				//do it the hard way
				for (uint32_t row_index = 0; row_index < rows_; ++row_index)
				{
					for (uint32_t col_index = 0; col_index < cols_; ++col_index)
					{
						for (uint32_t page_index = 0; page_index < pages_; ++page_index)
						{
							data_[MAT_INDEX(*this, row_index, col_index, page_index)] = static_cast<T>(0);
						}
					}
				}
			}
		}
		void Eye()
		{
			//this function is ROI-safe
			//this can only be run on both square matrices
			LOGASSERT(rows_ == cols_ && (pages_ == 1));
			Zero();
			for (uint32_t row_index = 0; row_index < rows_; ++row_index)
			{
				data_[MAT_INDEX(*this, row_index, row_index, 1)] = static_cast <T>(1);
			}
		}
		Matrix Roi(uint32_t row_start, int64_t num_rows, uint32_t col_start, int64_t num_cols, uint32_t page_start, int64_t num_pages)
		{
			//this function is ROI-safe
			//If you give this function 0 for num_* it will interpret that as "the rest of them"
			//if you give this function negative values for num_* it is interpret that as "the rest of them minus abs(num_*)"
			if (num_rows <= 0)
			{
				num_rows = rows_ - row_start + num_rows;
			}
			if (num_cols <= 0)
			{
				num_cols = cols_ - col_start + num_cols;
			}
			if (num_pages <= 0)
			{
				num_pages = pages_ - page_start + num_pages;
			}
			LOGASSERT(row_start + num_rows <= allocated_rows_);
			LOGASSERT(col_start + num_cols <= allocated_cols_);
			LOGASSERT(page_start + num_pages <= allocated_pages_);
			//Don't allow creating an ROI which extends outside the original ROI
			if (is_roi_)
			{
				if (row_start < roi_row_origin_ || col_start < roi_col_origin_ || page_start < roi_page_origin_ || 
					(row_start + num_rows) > rows_ || (col_start + num_cols) > cols_ || (page_start + num_pages > pages_))
				{
					LOGFATAL("Creating an ROI from an ROI, and the new ROI extends past the original ROI. Original ROI has size (%u,%u,%u). New ROI starts at (%u,%u,%u) and has size (%u,%u,%u)", rows_, cols_, pages_, row_start, col_start, page_start, num_rows, num_cols, num_pages);
				}
			}

			Matrix result;

			result.is_roi_ = true;
			result.data_ = data_;
			result.allocated_cells_ = 0;
			result.rows_ = (uint32_t)num_rows;
			result.cols_ = (uint32_t)num_cols;
			result.pages_ = (uint32_t)num_pages;
			result.roi_row_origin_ = row_start + roi_row_origin_;
			result.roi_col_origin_ = col_start + roi_col_origin_;
			result.roi_page_origin_ = page_start + roi_page_origin_;
			result.allocated_rows_ = allocated_rows_;
			result.allocated_cols_ = allocated_cols_;
			result.allocated_pages_ = allocated_pages_;
			result.format_ = format_;
			//note it is not required that row_start == 0 nor that rows_ == num_rows because the data can still be continuous in the middle of the allocated region
			if (col_start == 0 && page_start == 0 && allocated_cols_ == num_cols && allocated_pages_ == num_pages)
			{
				result.continuous_ = true;
			}
			else
			{
				result.continuous_ = false;
			}

			return result;
		}
		void PrintAscii(std::ostream& stream)
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
						stream << data_[MAT_INDEX(*this, row_index, col_index, page_index)];
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
		void DumpBinary(std::ostream& stream)
		{
			LOGASSERT(continuous_);
			//Note that row_*cols_*pages_ may or may not be == allocated_cells due to reallocation or an ROI starting at the origin
			//Note that *data here will never fail (even if data == NULL) becaue sizeof(*data_) is actually going to be looked up at compile time
			//The MAT_INDEX is added in case roi_row_origin_ != 0
			stream.write((const char*)data_ + MAT_INDEX(*this,0,0,0), rows_ * cols_ * pages_ * sizeof(*data_));
		}
		std::string Sprintf()
		{
			std::stringstream buf;
			PrintAscii(buf);
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
		enum PrintFmt
		{
			FMT_BIN,
			FMT_ASCII
		} ;
		void SetPrintFmt(PrintFmt format)
		{
			format_ = format;
		}
		PrintFmt GetPrintFmt()
		{
			return format_;
		}
		enum class ConvFlag
		{
			CONV_FLAG_ZERO_PAD = 0,
			CONV_FLAG_MIRROR = 1,
			CONV_FLAG_COPY = 2,
			CONV_FLAG_WRAPAROND = 3
		};
		template <class U>
		friend ion::Matrix<U> Convolve(ion::Matrix<U> mat, ion::Matrix<U> kernel, typename ion::Matrix<U>::ConvFlag flags);
		template <class U>
		friend bool indexInPad(Matrix<U> mat_padded, Matrix<U> kernel, uint32_t row, uint32_t col, uint32_t page);
		template <class U>
		friend void ElementwiseMultiply(Matrix<U> multiplicand, Matrix<U> multiplier, Matrix<U>& result);
		template <class U>
		friend U Sum(Matrix<U> augend);
	private:
		Matrix() { } //default construction is only allowed by the library so this is private
		//size of the non-roi matrix. It is guaranteed that (data_ + rows_*cols_*pages_) is the last element if !is_roi_
		uint32_t rows_;
		uint32_t cols_;
		uint32_t pages_;
		uint64_t allocated_cells_;
		//size of the roi matrix, if any. It is guaranteed that roi_rows_ <= rows_, etc.
		uint32_t roi_row_origin_;
		uint32_t roi_col_origin_;
		uint32_t roi_page_origin_;
		//these differ from rows_,cols_, and pages_ because it specifies the stride between rols/cols/pages (that is, for an ROI, this is >= rows_ cols_ pages_)
		//Note that it is NOT necessary that allocated_rows_*allocated_cols_*allocated_pages_ == allocated_cells_ (if not all the allocated data is being used)
		uint32_t allocated_rows_;
		uint32_t allocated_cols_;
		uint32_t allocated_pages_;
		bool     continuous_;
		bool     is_roi_;
		T* data_;
		PrintFmt format_;
	};
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
}; //namespace ion

template class ion::Matrix<double>;

#endif //ION_MATRIX_H_