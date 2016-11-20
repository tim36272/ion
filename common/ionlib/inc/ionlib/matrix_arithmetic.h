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
	T ion::Matrix<T>::Sum()
	{
		T sum = static_cast<T>(0);
		for (uint32_t row = 0; row < this->rows_; ++row)
		{
			for (uint32_t col = 0; col < this->cols_; ++col)
			{
				for (uint32_t page = 0; page < this->pages_; ++page)
				{
					sum += this->data_[MAT_INDEX(*this, row, col, page)];
				}
			}
		}
		return sum;
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreach_t foreach, ion::Matrix<T>* result)
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
					result->data_[MAT_INDEX(*result, row, col, page)] = foreach(data_[MAT_INDEX(*this, row, col, page)]);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPair_t foreach, const ion::Matrix<T>& rhs, ion::Matrix<T>* result)
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
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPair_t foreach, T constant, ion::Matrix<T>* result)
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
					result->data_[MAT_INDEX(*result, row, col, page)] = foreach(data_[MAT_INDEX(*this, row, col, page)], constant);
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Foreach(typename ion::Matrix<T>::foreachPair_t foreach, T* usrdata)
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
	inline T Multiply(T left, T right)
	{
		return left * right;
	}
	template <class T>
	void ion::Matrix<T>::ElementwiseMultiply(const Matrix<T>& multiplier, Matrix<T>* result)
	{
		//matrices must be the same shape
		//This function is ROI-safe
		LOGASSERT(rows_ == multiplier.rows_ && multiplier.rows_ == result->rows_);
		LOGASSERT(cols_ == multiplier.cols_ && multiplier.cols_ == result->cols_);
		LOGASSERT(pages_ == multiplier.pages_ && multiplier.pages_ == result->pages_);
		Foreach(&Multiply, multiplier, result);
	}
	template <class T>
	inline T Add(T left, T right)
	{
		return left + right;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::operator+(const ion::Matrix<T>& rhs)
	{
		//This function is ROI-safe
		//This function supports broadcast
		//matrices must be the same shape
		LOGASSERT(rhs.rows_ == rows_ || rhs.rows_ == 1 || rows_ == 1);
		LOGASSERT(rhs.cols_ == cols_ || rhs.cols_ == 1 || cols_ == 1);
		LOGASSERT(rhs.pages_ == pages_ || rhs.pages_ == 1 || pages_ == 1);
		uint32_t result_rows  = ion::Max(rhs.rows_, rows_);
		uint32_t result_cols  = ion::Max(rhs.cols_, cols_);
		uint32_t result_pages = ion::Max(rhs.pages_, pages_);

		ion::Matrix<T> result(result_rows, result_cols, result_pages);
		//check if this is a trivial addition or broadcast
		if (rhs.rows_ == rows_ && rhs.cols_ == cols_ && rhs.pages_ == pages_)
		{
			Foreach(&Add, rhs, &result);
		} else
		{
			uint32_t left_row_step = rows_ == 1 ? 0 : 1;
			uint32_t right_row_step = rhs.rows_ == 1 ? 0 : 1;
			uint32_t left_col_step = cols_ == 1 ? 0 : 1;
			uint32_t right_col_step = rhs.cols_ == 1 ? 0 : 1;
			uint32_t left_page_step = pages_ == 1 ? 0 : 1;
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
						result.At(row_index, col_index, page_index) = this->At(left_row_index, left_col_index, left_page_index) + rhs.At(right_row_index, right_col_index, right_page_index);
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
	inline T Subtract(T left, T right)
	{
		return left - right;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::operator-(const ion::Matrix<T>& rhs)
	{
		//This function is ROI-safe
		//matrices must be the same shape
		LOGASSERT(rhs.rows_ == rows_);
		LOGASSERT(rhs.cols_ == cols_);
		LOGASSERT(rhs.pages_ == pages_);
		ion::Matrix<T> result(rhs.rows_, rhs.cols_, rhs.pages_);
		Foreach(&Subtract, rhs, &result);
		return result;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::operator*(const ion::Matrix<T>& rhs)
	{
		LOGFATAL("Not yet implemented");
		Matrix<T> result;
		return result;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::operator*(T rhs)
	{
		ion::Matrix<T> result(rows_, cols_, pages_);
		Foreach(&Multiply, rhs, &result);
		return result;
	}
	template <class T>
	T ion::Matrix<T>::Dot(const ion::Matrix<T>& rhs)
	{
		//must be single-dimensional
		LOGASSERT(((rows_ == 1) ^ (cols_ == 1)) ^ (pages_ == 1));
		LOGASSERT(((rhs.rows_ == 1) ^ (rhs.cols_ == 1)) ^ (rhs.pages_ == 1));
		T result = static_cast<T>(0);
		//interestingly, if the matrix is continuous we can do it by index
		if (continuous_ && rhs.continuous_)
		{
			for (uint32_t index = 0; index < rhs.allocated_cells_; ++index)
			{
				result += rhs.data_[rhs.roi_row_origin_ + index] * data_[roi_row_origin_ + index];
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
				result += data_[MAT_INDEX(*this, row, col, page)] * rhs.data_[MAT_INDEX(rhs, rhs_row, rhs_col, rhs_page)];
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
	void ion::Matrix<T>::Transpose(ion::Matrix<T>* result)
	{
		//this function is ROI-safe
		//this function is inplace safe
		//this could be done for 3D matrices but I don't support that yet
		LOGASSERT(pages_ == 0);
	}
	template <class T>
	void ion::Matrix<T>::Inverse(ion::Matrix<T>* result )
	{
		LOGFATAL("Not yet implemented");
	}
	template <class T>
	T ion::Matrix<T>::Determinent()
	{
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
	T ion::Matrix<T>::Max()
	{
		T result;
		Foreach(&ion::Max, &result);
		return result;
	}
	//explicit instantiations

} //namespace ion
