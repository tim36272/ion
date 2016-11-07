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
	bool indexInPad(Matrix<T> mat_padded, Matrix<T> kernel, uint32_t row, uint32_t col, uint32_t page)
	{
		if (row > kernel.rows_ / 2 && row < (mat_padded.rows_ - kernel.rows_/2))
		{
			if (col > kernel.cols_ / 2 && col < (mat_padded.cols_ - kernel.cols_/2))
			{
				if (page > kernel.pages_ / 2 && page < (mat_padded.pages_ - kernel.pages_ / 2))
				{
					return false;
				} else
				{
					return true;
				}
			} else
			{
				return true;
			}
		} else
		{
			return true;
		}
	}
	template <class T>
	T Sum(Matrix<T> augend)
	{
		T sum = static_cast<T>(0);
		for (uint32_t row = 0; row < augend.rows_; ++row)
		{
			for (uint32_t col = 0; col < augend.cols_; ++col)
			{
				for (uint32_t page = 0; page < augend.pages_; ++page)
				{
					sum += augend.data_[MAT_INDEX(augend,row, col, page)];
				}
			}
		}
		return sum;
	}
	template <class T>
	void ElementwiseMultiply(Matrix<T> multiplicand, Matrix<T> multiplier, Matrix<T>& result)
	{
		//This function is ROI-safe
		LOGASSERT(multiplicand.rows_ == multiplier.rows_ && multiplier.rows_ == result.rows_);
		LOGASSERT(multiplicand.cols_ == multiplier.cols_ && multiplier.cols_ == result.cols_);
		LOGASSERT(multiplicand.pages_ == multiplier.pages_ && multiplier.pages_ == result.pages_);
		for (uint32_t row = 0; row < multiplicand.rows_; ++row)
		{
			for (uint32_t col = 0; col < multiplicand.cols_; ++col)
			{
				for (uint32_t page = 0; page < multiplicand.pages_; ++page)
				{
					result.data_[MAT_INDEX(result,row, col, page)] = multiplicand.data_[MAT_INDEX(multiplicand,row, col, page)] * multiplier.data_[MAT_INDEX(multiplier,row, col, page)];
				}
			}
		}
	}
	template <class T>
	ion::Matrix<T> Convolve(ion::Matrix<T> mat, ion::Matrix<T> kernel, typename ion::Matrix<T>::ConvFlag flags)
	{
		LOGASSERT(kernel.rows_ % 2 == 1);
		LOGASSERT(kernel.cols_ % 2 == 1);
		LOGASSERT(kernel.pages_ % 2 == 1);
		//create a temporary matrix which is padded appropriately
		Matrix<T> mat_padded(mat.rows_ + kernel.rows_ - 1, mat.cols_ + kernel.cols_ - 1, mat.pages_ + kernel.pages_ - 1);
		if (flags == Matrix<T>::ConvFlag::CONV_FLAG_ZERO_PAD)
		{
			mat_padded.Zero();
		}
		//copy the original mat to the padded one
		Matrix<T> mat_padded_roi = mat_padded.Roi(kernel.rows_ / 2,  -(int64_t)kernel.rows_ / 2,
												  kernel.cols_ / 2,  -(int64_t)kernel.cols_ / 2,
												  kernel.pages_ / 2, -(int64_t)kernel.pages_ / 2);

		mat.DeepCopyTo(mat_padded_roi);
		//fill in the edges of mat_padded
		if (flags != Matrix<T>::ConvFlag::CONV_FLAG_ZERO_PAD)
		{
			for (uint32_t row = 0; row < ceil(mat_padded.rows_/2.0); ++row)
			{
				for (uint32_t col = 0; col < ceil(mat_padded.cols_/2.0); ++col)
				{
					for (uint32_t page = 0; page < ceil(mat_padded.pages_/2.0); ++page)
					{
						if (indexInPad(mat_padded, kernel, row, col, page))
						{
							switch (flags)
							{
								case typename Matrix<T>::ConvFlag::CONV_FLAG_MIRROR:
								{
									uint32_t reverse_dst_row = mat_padded.rows_ - row - 1;
									uint32_t reverse_dst_col = mat_padded.cols_ - col - 1;
									uint32_t reverse_dst_page = mat_padded.pages_ - page - 1;
									uint32_t src_row = kernel.rows_ / 2 + MAX((int64_t)row - 1, 0);
									uint32_t src_col = kernel.cols_ / 2 + MAX((int64_t)col - 1, 0);
									uint32_t src_page = kernel.pages_ / 2 + MAX((int64_t)page - 1, 0);
									uint32_t reverse_src_row = mat_padded.rows_ - src_row - 1;
									uint32_t reverse_src_col = mat_padded.cols_ - src_col - 1;
									uint32_t reverse_src_page = mat_padded.pages_ - src_page - 1;

									//corner [0,0,0]
									mat_padded.data_[MAT_INDEX(mat_padded, row, col, page)] = mat_padded.data_[MAT_INDEX(mat_padded, src_row, src_col, src_page)];
									//corner [0,0,1]
									mat_padded.data_[MAT_INDEX(mat_padded, row, col, reverse_dst_page)] = mat_padded.data_[MAT_INDEX(mat_padded, src_row, src_col, reverse_src_page)];
									//corner [0,1,0]
									mat_padded.data_[MAT_INDEX(mat_padded, row, reverse_dst_col, page)] = mat_padded.data_[MAT_INDEX(mat_padded, src_row, reverse_src_col, src_page)];
									//corner [0,1,1]
									mat_padded.data_[MAT_INDEX(mat_padded, row, reverse_dst_col, reverse_dst_page)] = mat_padded.data_[MAT_INDEX(mat_padded, src_row, reverse_src_col, reverse_src_page)];
									//corner [1,0,0]
									mat_padded.data_[MAT_INDEX(mat_padded, reverse_dst_row, col, page)] = mat_padded.data_[MAT_INDEX(mat_padded, reverse_src_row, src_col, src_page)];
									//corner [1,0,1]
									mat_padded.data_[MAT_INDEX(mat_padded, reverse_dst_row, col, reverse_dst_page)] = mat_padded.data_[MAT_INDEX(mat_padded, reverse_src_row, src_col, reverse_src_page)];
									//corner [1,1,0]
									mat_padded.data_[MAT_INDEX(mat_padded, reverse_dst_row, reverse_dst_col, page)] = mat_padded.data_[MAT_INDEX(mat_padded, reverse_src_row, reverse_src_col, src_page)];
									//corner [1,1,1]
									mat_padded.data_[MAT_INDEX(mat_padded, reverse_dst_row, reverse_dst_col, reverse_dst_page)] = mat_padded.data_[MAT_INDEX(mat_padded, reverse_src_row, reverse_src_col, reverse_src_page)];
								}
									break;
								case typename Matrix<T>::ConvFlag::CONV_FLAG_COPY:

									break;
								case typename Matrix<T>::ConvFlag::CONV_FLAG_WRAPAROND:
									LOGFATAL("Not yet implemented");
									break;

							}
						}
					}
				}
			}
		}
		//create output
		Matrix<T> output(mat.rows_, mat.cols_, mat.pages_);
		Matrix<T> temp(kernel.rows_, kernel.cols_, kernel.pages_);
		for (uint32_t row = 0; row < output.rows_; ++row)
		{
			for (uint32_t col = 0; col < output.cols_; ++col)
			{
				for (uint32_t page = 0; page < output.pages_; ++page)
				{
					//now row,col,page is centered around the cell to convolve
					//make a region of interest around this cell
					Matrix<T> src_roi = mat_padded.Roi(row, kernel.rows_, col, kernel.cols_, page, kernel.pages_);
					ElementwiseMultiply(src_roi, kernel, temp);
					output.data_[MAT_INDEX(output, row, col, page)] = Sum(temp);
				}
			}
		}
		return output;
	}
	template Matrix<double> Convolve(Matrix<double> mat, Matrix<double> kernel, Matrix<double>::ConvFlag flags);
} //namespace ion
