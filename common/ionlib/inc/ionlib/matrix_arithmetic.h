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
	T ion::Matrix<T>::Sum() const
	{
		T sum = static_cast<T>(0);
		for (uint32_t row = 0; row < this->rows_; ++row)
		{
			for (uint32_t col = 0; col < this->cols_; ++col)
			{
				for (uint32_t page = 0; page < this->pages_; ++page)
				{
					//this does not use ion::Matrix::At for performance reasons
					sum += data_[MAT_INDEX(*this,row, col, page)];
				}
			}
		}
		return sum;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::SumRows() const
	{
		ion::Matrix<T> result(rows_, 1, 1);
		for (uint32_t row = 0; row < rows_; ++row)
		{
			ion::Matrix<T> in_roi = Roi(row, 1, 0 ,0, 0, 0);
			result.At(row) = in_roi.Sum();
		}
		return result;
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreach_t foreach, ion::Matrix<T>* result) const
	{
		//this function is inplace safe
		//make sure result is the right size
		result->Resize(rows_, cols_, pages_);
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					result->data_[MAT_INDEX(*result,row, col, page)] = foreach(data_[MAT_INDEX(*this,row, col, page)]);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPair_t foreach, const ion::Matrix<T>& rhs, ion::Matrix<T>* result) const
	{
		//this function is inplace safe
		//make sure result is the right size
		result->Resize(rows_, cols_, pages_);
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					result->data_[MAT_INDEX(*result, row, col, page)] = foreach(data_[MAT_INDEX(*this, row, col, page)], rhs.data_[MAT_INDEX(rhs, row, col, page)]);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPair_t foreach, T constant, ion::Matrix<T>* result) const
	{
		//this function is inplace safe
		LOGASSERT(result->rows_ == rows_ && result->cols_ == cols_ && result->pages_ == pages_);
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					result->data_[MAT_INDEX(*result, row, col, page)] = foreach(data_[MAT_INDEX(*this, row, col, page)], constant);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPair_t foreach, T* usrdata) const
	{
		//this function is inplace safe
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					*usrdata = foreach(data_[MAT_INDEX(*this, row, col, page)], *usrdata);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPairUsrdata_t foreach, void* usrdata, ion::Matrix<T> *result) const
	{
		LOGASSERT(result->rows_ == rows_ && result->cols_ == cols_ && result->pages_ == pages_);
		//this function is inplace safe
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					result->data_[MAT_INDEX(*result, row, col, page)] = foreach(data_[MAT_INDEX(*this, row, col, page)], usrdata);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::ElementwiseMultiply(const Matrix<T>& multiplier, Matrix<T>* result) const
	{
		//matrices must be the same shape
		//This function is ROI-safe
		LOGASSERT(rows_ == multiplier.rows_ && multiplier.rows_ == result->rows_);
		LOGASSERT(cols_ == multiplier.cols_ && multiplier.cols_ == result->cols_);
		LOGASSERT(pages_ == multiplier.pages_ && multiplier.pages_ == result->pages_);
		size_t this_idx, multiplier_idx, result_idx;
		//This does not use Foreach for performance reasons
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					this_idx = MAT_INDEX(*this, row, col, page);
					multiplier_idx = MAT_INDEX(multiplier, row, col, page);
					result_idx = MAT_INDEX(*result, row, col, page);
					//this does not use ion::Matrix::At for performance reasons
					result->data_[result_idx] = data_[this_idx] * multiplier.data_[multiplier_idx];
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::ElementwiseMultiplyRotated(const Matrix<T>& multiplier, Matrix<T>* result) const
	{
		//matrices must be the same shape
		//This function is ROI-safe
		LOGASSERT(rows_ == multiplier.rows_ && multiplier.rows_ == result->rows_);
		LOGASSERT(cols_ == multiplier.cols_ && multiplier.cols_ == result->cols_);
		LOGASSERT(pages_ == multiplier.pages_ && multiplier.pages_ == result->pages_);
		size_t this_idx, multiplier_idx, result_idx;
		//This does not use Foreach for performance reasons
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					this_idx = MAT_INDEX(*this, row, col, page);
					multiplier_idx = MAT_INDEX(multiplier, rows_-row-1, cols_-col-1, pages_-page-1);
					result_idx = MAT_INDEX(*result, row, col, page);
					//this does not use ion::Matrix::At for performance reasons
					result->data_[result_idx] = data_[this_idx] * multiplier.data_[multiplier_idx];
				}
			}
		}
	}

	template <class T>
	ion::Matrix<T> operator+(const ion::Matrix<T>& lhs, const ion::Matrix<T>& rhs)
	{
		//This function is ROI-safe
		//This function supports broadcast
		//matrices must be the same shape
		LOGASSERT(rhs.rows_  == lhs.rows_ || rhs.rows_   == 1 || lhs.rows_ == 1);
		LOGASSERT(rhs.cols_  == lhs.cols_ || rhs.cols_   == 1 || lhs.cols_ == 1);
		LOGASSERT(rhs.pages_ == lhs.pages_ || rhs.pages_ == 1 || lhs.pages_ == 1);
		uint32_t result_rows  = ion::Max(rhs.rows_,  lhs.rows_);
		uint32_t result_cols  = ion::Max(rhs.cols_,  lhs.cols_);
		uint32_t result_pages = ion::Max(rhs.pages_, lhs.pages_);

		ion::Matrix<T> result(result_rows, result_cols, result_pages);
		//check if this is a trivial addition or broadcast
		if (rhs.rows_ == lhs.rows_ && rhs.cols_ == lhs.cols_ && rhs.pages_ == lhs.pages_)
		{
			lhs.Foreach(&ion::Add, rhs, &result);
		} else
		{
			uint32_t left_row_step = lhs.rows_ == 1 ? 0 : 1;
			uint32_t right_row_step = rhs.rows_ == 1 ? 0 : 1;
			uint32_t left_col_step = lhs.cols_ == 1 ? 0 : 1;
			uint32_t right_col_step = rhs.cols_ == 1 ? 0 : 1;
			uint32_t left_page_step = lhs.pages_ == 1 ? 0 : 1;
			uint32_t right_page_step = rhs.pages_ == 1 ? 0 : 1;

			uint32_t left_row_index = 0;
			uint32_t right_row_index = 0;
			for (uint32_t row_index = 0; row_index < result_rows; ++row_index)
			{
				uint32_t left_col_index = 0;
				uint32_t right_col_index = 0;
				for (uint32_t col_index = 0; col_index < result_cols; ++col_index)
				{
					uint32_t left_page_index = 0;
					uint32_t right_page_index = 0;
					for (uint32_t page_index = 0; page_index < result_pages; ++page_index)
					{
						result.At(row_index, col_index, page_index) = lhs.At(left_row_index, left_col_index, left_page_index) + rhs.At(right_row_index, right_col_index, right_page_index);
						left_page_index += left_page_step;
						right_page_index += right_page_step;
					}
					left_col_index += left_col_step;
					right_col_index += right_col_step;
				}
				left_row_index += left_row_step;
				right_row_index += right_row_step;
			}
		}

		return result;
	}
	template <class T>
	ion::Matrix<T> operator+(const ion::Matrix<T>& lhs, T rhs)
	{
		ion::Matrix<T> result(lhs.rows_, lhs.cols_, lhs.pages_);
		lhs.Foreach(&ion::Add, rhs, &result);
		return result;
	}
	template <class T>
	ion::Matrix<T> operator-(const ion::Matrix<T>& lhs, const ion::Matrix<T>& rhs)
	{
		//This function is ROI-safe
		//matrices must be the same shape
		LOGASSERT(rhs.rows_  == lhs.rows_);
		LOGASSERT(rhs.cols_  == lhs.cols_);
		LOGASSERT(rhs.pages_ == lhs.pages_);
		ion::Matrix<T> result(rhs.rows_, rhs.cols_, rhs.pages_);
		lhs.Foreach(&ion::Subtract, rhs, &result);
		return result;
	}
	template <class T>
	ion::Matrix<T> operator-(T lhs, const Matrix<T>& rhs)
	{
		ion::Matrix<T> result(rhs.rows_, rhs.cols_, rhs.pages_);
		lhs.Foreach(&ion::SubtractSwap, lhs, &result);
		return result;
	}
	template <class T>
	ion::Matrix<T> operator*(const ion::Matrix<T>& lhs, const ion::Matrix<T>& rhs)
	{
		uint32_t c, d, k;
		T sum = static_cast<T>(0);
		LOGASSERT(rhs.pages_ == 1 && lhs.pages_ == 1);
		LOGASSERT(lhs.cols_ == rhs.rows_);
		Matrix<T> result(lhs.rows_,rhs.cols_);
		for (c = 0; c < lhs.rows_; c++)
		{
			for (d = 0; d < rhs.cols_; d++)
			{
				for (k = 0; k < rhs.rows_; k++)
				{

					sum = sum + lhs.data_[MAT_INDEX(lhs,c,k,0)] * rhs.data_[MAT_INDEX(rhs,k,d,0)];
				}

				result.data_[MAT_INDEX(result,c,d,0)] = sum;
				sum = static_cast<T>(0);
			}
		}
		return result;
	}
	template <class T>
	ion::Matrix<T> operator*(const ion::Matrix<T>& lhs, T rhs)
	{
		ion::Matrix<T> result(lhs.rows_, lhs.cols_, lhs.pages_);
		lhs.Foreach(&ion::Multiply, rhs, &result);
		return result;
	}
	template <class T>
	ion::Matrix<T> operator/(const ion::Matrix<T>& lhs, T rhs)
	{
		ion::Matrix<T> result(lhs.rows_, lhs.cols_, lhs.pages_);
		lhs.Foreach(&ion::Divide, rhs, &result);
		return result;
	}
	template <class T>
	ion::Matrix<T> operator/(T lhs, const ion::Matrix<T>& rhs)
	{
		ion::Matrix<T> result(rhs.rows_, rhs.cols_, rhs.pages_);
		lhs.Foreach(&ion::SwapDivide, lhs, &result);
		return result;
	}
	template <class T>
	T ion::Matrix<T>::Dot(const ion::Matrix<T>& rhs) const
	{
		//must be single-dimensional
		LOGASSERT(((rows_ == 1) ^ (cols_ == 1)) ^ (pages_ == 1));
		LOGASSERT(((rhs.rows_ == 1) ^ (rhs.cols_ == 1)) ^ (rhs.pages_ == 1));
		T result = static_cast<T>(0);
		//interestingly, if the matrix is continuous we can do it by index
		if (contiguous_ && rhs.contiguous_)
		{
			for (uint32_t index = 0; index < rhs.allocated_cells_; ++index)
			{
				result += rhs.data_[MAT_INDEX(rhs,0,0,0) + index] * data_[(*this, 0, 0, 0) + index];
			}
		} else
		{
			uint32_t row_step = 0, col_step = 0, page_step = 0;
			uint32_t rhs_row_step = 0, rhs_col_step = 0, rhs_page_step = 0;
			if (rows_ > 1)
			{
				row_step = 1;
			} else if (rows_ > 1)
			{
				row_step = 1;
			} else //note this allows support for 1x1x1 matrices
			{
				page_step = 1; 
			}
			if (rhs.rows_ > 1)
			{
				rhs_row_step = 1;
			} else if (rhs.rows_ > 1)
			{
				rhs_row_step = 1;
			} else //note this allows support for 1x1x1 matrices
			{
				rhs_page_step = 1;
			}
			uint32_t elements_in_vector = ion::Max(ion::Max(row_step, col_step), page_step);
			uint32_t elements_in_rhs_vector = ion::Max(ion::Max(rhs_row_step, rhs_col_step), rhs_page_step);
			LOGASSERT(elements_in_rhs_vector == elements_in_vector);
			uint32_t row = 0, col = 0, page = 0;
			uint32_t rhs_row = 0, rhs_col = 0, rhs_page = 0;
			for (uint32_t index = 0; index < elements_in_vector; ++index)
			{
				result += At(row, col, page) * rhs.At(rhs_row, rhs_col, rhs_page);
				row += row_step;
				col += col_step;
				page += page_step;
				rhs_row += rhs_row_step;
				rhs_col += rhs_col_step;
				rhs_page += rhs_page_step;
			}
		}
		return result;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::DotAsColumns(const Matrix<T>& rhs) const
	{
		//use the Matlab convention where the dot product of two matrices treats them as column vectors and multiplies them that way
		//I'm not sure what to do if there are pages, so for now force them to size=1
		LOGASSERT(pages_ == 1 && rhs.pages_ == 1);
		LOGASSERT(rows_ == rhs.rows_ && cols_ == rhs.cols_);
		ion::Matrix<T> result(1,cols_);
		for (uint32_t col = 0; col < cols_; ++col)
		{
			ion::Matrix<T> lhs_roi = Roi(0, 0, col, 1);
			const ion::Matrix<T> rhs_roi = rhs.Roi(0, 0, col, 1);
			result.At(0,col) = lhs_roi.Dot(rhs_roi);
		}
		return result;
	}

	template <class T>
	void ion::Matrix<T>::Transpose(ion::Matrix<T>* result)
	{
		//this function is ROI-safe
		//this function is inplace safe
		//this could be done for 3D matrices but I don't support that yet
		LOGASSERT(pages_ == 1 && result->pages_ == 1);
		LOGFATAL("No yet implemented");
	}
	template <class T>
	void ion::Matrix<T>::Inverse(ion::Matrix<T>* result )
	{
		//this could be done for 3D matrices but I don't support that yet

		LOGASSERT(pages_ == 1 && result->pages_ == 1);
		LOGFATAL("Not yet implemented");
	}
	template <class T>
	T ion::Matrix<T>::Determinent() const
	{
		//this could be done for 3D matrices but I don't support that yet
		LOGASSERT(pages_ == 1);
		LOGFATAL("Not yet implemented");
		T result = static_cast<T>(0);
		return result;
	}
	template <class T>
	T Max(T lhs, T rhs)
	{
		return (lhs > rhs) ? lhs : rhs;
	}
	template <class T>
	T ion::Matrix<T>::Max() const
	{
		T result = std::numeric_limits<T>::lowest();
		Foreach(&ion::Max, &result);
		return result;
	}
	template <class T>
	T ion::Matrix<T>::Mean() const
	{
		return Sum() / static_cast<T>(rows_ * cols_ * pages_);
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::Log() const
	{
		ion::Matrix<T> result(rows_, cols_, pages_);
		Foreach(&ion::Log, &result);
		return result;
	}
	//computes argmax by flattening the array first
	template <class T>
	uint64_t ion::Matrix<T>::Argmax() const
	{
		T max = std::numeric_limits<T>::lowest();
		uint32_t max_row=0, max_col=0, max_page=0; //assigned 0 in case the entire matrix is full of std::numeric_limits<T>::lowest()
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					if (At(row, col, page) > max)
					{
						max = At(row, col, page);
						max_row = row;
						max_col = col;
						max_page = page;
					}
				}
			}
		}
		return max_row * pages_ * cols_ + max_col * pages_ + max_page;
	}
	template <class T>
	ion::Matrix<uint32_t> ion::Matrix<T>::Argmax(uint32_t dim) const
	{
		//I don't know what this means for a 3D matrix so disallow it:
		LOGASSERT(pages_ == 1);
		LOGASSERT(dim == 1);

		ion::Matrix<uint32_t> result(rows_);
		for (uint32_t row = 0; row < rows_; ++row)
		{
			uint32_t max_pos = 0;
			T max = std::numeric_limits<T>::lowest();
			for (uint32_t col = 0; col < cols_; ++col)
			{
				if (At(row, col) > max)
				{
					max = At(row, col);
					max_pos = col;
				}
			}
			result.At(row) = max_pos;
		}
		return result;
	}
	template <class T>
	uint64_t ion::Matrix<T>::Countif(typename ion::Matrix<T>::foreachPairBool_t foreach, T constant) const
	{
		uint64_t sum = 0;
		//this function is inplace safe
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					if (foreach(data_[MAT_INDEX(*this, row, col, page)], constant))
					{
						sum++;
					}
				}
			}
		}
		return sum;
	}

} //namespace ion
