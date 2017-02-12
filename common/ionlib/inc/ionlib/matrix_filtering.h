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
#define MAT_ROI2D_FASTEST(mat,row,col,dst) {\
dst.roi_row_origin_ = row + mat.roi_row_origin_;\
dst.roi_col_origin_ = col + mat.roi_col_origin_;\
} 
namespace ion
{
	template <class T>
	bool indexInPad(const Matrix<T>& mat_padded, const Matrix<T>& kernel, uint32_t row, uint32_t col, uint32_t page)
	{
		if (row > kernel.rows_ / 2 && row < (mat_padded.rows_ - kernel.rows_ / 2))
		{
			if (col > kernel.cols_ / 2 && col < (mat_padded.cols_ - kernel.cols_ / 2))
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
	ion::Matrix<T> Convolve(const ion::Matrix<T>& mat, const ion::Matrix<T>& kernel, typename ion::Matrix<T>::ConvFlag flags)
	{
		LOGASSERT(kernel.rows_ % 2 == 1);
		LOGASSERT(kernel.cols_ % 2 == 1);
		LOGASSERT(kernel.pages_ % 2 == 1);
		//create a temporary matrix which is padded appropriately
		uint32_t pad_rows, pad_cols, pad_pages;
		if ((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_NO_PAD)
		{
			//the output will just be shrunk
			pad_rows = 0;
			pad_cols = 0;
			pad_pages =0;
		} else
		{
			pad_rows = kernel.rows_ - 1;
			pad_cols = kernel.cols_ - 1;
			pad_pages = kernel.pages_ - 1;
		}
		Matrix<T> mat_padded(mat.rows_ + pad_rows,mat.cols_ + pad_cols,mat.pages_ + pad_pages);
		if ((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_ZERO_PAD)
		{
			mat_padded.Zero();
		}
		//copy the original mat to the padded one
		Matrix<T> mat_padded_roi = mat_padded.Roi(pad_rows/2, -(int64_t)pad_rows / 2,
												  pad_cols/2, -(int64_t)pad_cols / 2,
												  pad_pages/ 2, -(int64_t)pad_pages / 2);

		mat.DeepCopyTo(mat_padded_roi);
		//fill in the edges of mat_padded
		if (!((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_ZERO_PAD) && 
			!((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_NO_PAD))
		{
			for (uint32_t row = 0; row < ceil(mat_padded.rows_ / 2.0); ++row)
			{
				for (uint32_t col = 0; col < ceil(mat_padded.cols_ / 2.0); ++col)
				{
					for (uint32_t page = 0; page < ceil(mat_padded.pages_ / 2.0); ++page)
					{
						if (indexInPad(mat_padded, kernel, row, col, page))
						{
							if((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_MIRROR)
							{
								uint32_t reverse_dst_row = mat_padded.rows_ - row - 1;
								uint32_t reverse_dst_col = mat_padded.cols_ - col - 1;
								uint32_t reverse_dst_page = mat_padded.pages_ - page - 1;
								uint32_t src_row = kernel.rows_ / 2 +   static_cast<uint32_t>(MAX((int64_t)row - 1, 0));
								uint32_t src_col = kernel.cols_ / 2 +   static_cast<uint32_t>(MAX((int64_t)col - 1, 0));
								uint32_t src_page = kernel.pages_ / 2 + static_cast<uint32_t>(MAX((int64_t)page - 1, 0));
								uint32_t reverse_src_row = mat_padded.rows_ - src_row - 1;
								uint32_t reverse_src_col = mat_padded.cols_ - src_col - 1;
								uint32_t reverse_src_page = mat_padded.pages_ - src_page - 1;

								//corner [0,0,0]
								mat_padded.At(row, col, page) = mat_padded.At(src_row, src_col, src_page);
								//corner [0,0,1]
								mat_padded.At(row, col, reverse_dst_page) = mat_padded.At(src_row, src_col, reverse_src_page);
								//corner [0,1,0]
								mat_padded.At(row, reverse_dst_col, page) = mat_padded.At(src_row, reverse_src_col, src_page);
								//corner [0,1,1]
								mat_padded.At(row, reverse_dst_col, reverse_dst_page) = mat_padded.At(src_row, reverse_src_col, reverse_src_page);
								//corner [1,0,0]
								mat_padded.At(reverse_dst_row, col, page) = mat_padded.At(reverse_src_row, src_col, src_page);
								//corner [1,0,1]
								mat_padded.At(reverse_dst_row, col, reverse_dst_page) = mat_padded.At(reverse_src_row, src_col, reverse_src_page);
								//corner [1,1,0]
								mat_padded.At(reverse_dst_row, reverse_dst_col, page) = mat_padded.At(reverse_src_row, reverse_src_col, src_page);
								//corner [1,1,1]
								mat_padded.At(reverse_dst_row, reverse_dst_col, reverse_dst_page) = mat_padded.At(reverse_src_row, reverse_src_col, reverse_src_page);
							}
							else
							{ //if ((flags & Matrix<T>::ConvFlag::CONV_FLAG_COPY) || (flasg & Matrix<T>::ConvFlag::CONV_FLAG_WRAPAROND))
								LOGFATAL("Not yet implemented");
							}
						}
					}
				}
			}
		}
		//create output
		//Here's what the size means: base size is mat, then in each dimensions we either shrink it if there is no pad or don't shrink it if there is a pad
		Matrix<T> output(mat.rows_ - (kernel.rows_ - (pad_rows + 1)), mat.cols_ - (kernel.cols_ - (pad_cols+1)), mat.pages_ - (kernel.pages_ - (pad_pages+1)));
		if ((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_SPARSE_Z)
		{
			//this is only done if Z is sparse because otherwise every cell is guaranteed to be filled
			output.Zero();
		}
		Matrix<T> src_roi = mat_padded.Roi(0, kernel.rows_,0,kernel.cols_);
		const uint32_t row_max = output.rows_;
		const uint32_t col_max = output.cols_;
		const uint32_t page_max = output.pages_;
		for (uint32_t row = 0; row < row_max; ++row)
		{
			for (uint32_t col = 0; col < col_max; ++col)
			{
				if ((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_SPARSE_Z)
				{
					//check if this page is 0
					bool all_zero = true;
					for (uint32_t page = 0; page < page_max; ++page)
					{
						if (mat_padded.At(row, col, page) != 0.0)
						{
							all_zero = false;
							break;
						}
					}
					if (all_zero)
					{
						continue;
					}
				}
				for (uint32_t page = 0; page < page_max; ++page)
				{
					//now row,col,page is centered around the cell to convolve
					//make a region of interest around this cell
					if (kernel.pages_ == 1)
					{
						MAT_ROI2D_FASTEST(mat_padded, row, col, src_roi);
					} else
					{
						mat_padded.Roi_ReallyFast(row, kernel.rows_, col, kernel.cols_, page, kernel.pages_, &src_roi);
					}
					//this doesn't use At for performance reasons
					if (kernel.pages_ == 1)
					{
						output.data_[MAT_INDEX(output, row, col, page)] = src_roi.ElementwiseMultiplyRotatedWithSumFast2D(kernel);
					} else
					{
						output.data_[MAT_INDEX(output, row, col, page)] = src_roi.ElementwiseMultiplyRotatedWithSumFast(kernel);
					}
				}
			}
		}

		return output;
	}

	template <class T>
	ion::Matrix<T> ConvolveDryRun(const ion::Matrix<T>& mat, const ion::Matrix<T>& kernel, typename ion::Matrix<T>::ConvFlag flags)
	{
		LOGASSERT(kernel.rows_ % 2 == 1);
		LOGASSERT(kernel.cols_ % 2 == 1);
		LOGASSERT(kernel.pages_ % 2 == 1);
		//create a temporary matrix which is padded appropriately
		uint32_t pad_rows, pad_cols, pad_pages;
		if ((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_NO_PAD)
		{
			//the output will just be shrunk
			pad_rows = 0;
			pad_cols = 0;
			pad_pages = 0;
		} else
		{
			pad_rows = kernel.rows_ - 1;
			pad_cols = kernel.cols_ - 1;
			pad_pages = kernel.pages_ - 1;
		}
		
		//create output
		//Here's what the size means: base size is mat, then in each dimensions we either shrink it if there is no pad or don't shrink it if there is a pad
		Matrix<T> output(mat.rows_ - (kernel.rows_ - (pad_rows + 1)), mat.cols_ - (kernel.cols_ - (pad_cols + 1)), mat.pages_ - (kernel.pages_ - (pad_pages + 1)));
		
		return output;
	}
	template <class T>
	ion::Matrix<T> MaxPool(const ion::Matrix<T>& mat, uint32_t pool_size)
	{
		uint32_t result_rows  = ion::Max(1U, mat.rows_ / pool_size);
		uint32_t result_cols  = ion::Max(1U, mat.cols_ / pool_size);
		uint32_t result_pages = ion::Max(1U, mat.pages_ / pool_size);
		
		//handle smaller than 3D mats:
		const uint32_t row_step = (mat.rows_ == 1) ? (1) : (pool_size);
		const uint32_t col_step = (mat.cols_ == 1) ? (1) : (pool_size);
		const uint32_t page_step = (mat.pages_ == 1) ? (1) : (pool_size);
		ion::Matrix<T> result(result_rows, result_cols, result_pages);
		Matrix<T> roi = mat.Roi(0, 0,0);
		const uint32_t row_max = (mat.rows_ - row_step + 1);
		const uint32_t col_max = (mat.cols_ - col_step + 1);
		const uint32_t page_max = (mat.pages_ - page_step + 1);
		for (uint32_t row = 0; row < row_max; row += row_step)
		{
			for (uint32_t col = 0; col < col_max; col += col_step)
			{
				for (uint32_t page = 0; page < page_max; page += page_step)
				{
					//create ROI for this macrovoxel
					mat.Roi_Fast(row, row_step, col, col_step, page, page_step, &roi);
					//this doesn't use ion::Matrix::At for performance reasons
					result.data_[MAT_INDEX(result,row / pool_size, col / pool_size, page / pool_size)] = roi.Max();
				}
			}
		}
		return result;
	}
	template <class T>
	ion::Matrix<T> Softmax(const ion::Matrix<T>& mat)
	{
		//I don't know how to define this for a 3D matrix, for now make it 2D
		LOGASSERT(mat.pages_ == 1);

		//compute the result rowwise
		ion::Matrix<T> result(mat.rows_,mat.cols_);
		for (uint32_t row = 0; row < mat.rows_; ++row)
		{
			//Compute the sum of exp(mat)
			ion::Matrix<T> temp(1, mat.cols_);
			T temp_sum = mat.Roi(row, 1).Abs().Sum();
			if (temp_sum == static_cast<T>(0))
			{
				temp_sum = static_cast<T>(1);
			}
			(mat.Roi(row,1) / temp_sum).Foreach(&ion::Exp, &temp);
			T sum = temp.Sum();
			//Calculate softmax
			ion::Matrix<T> result_roi = result.Roi(row, 1);
			temp.Foreach(&ion::Divide, sum, &result_roi);
		}
		return result;
	}
	template<class T>
	ion::Matrix<T> ion::Matrix<T>::ResampleBilerp(uint32_t new_rows, uint32_t new_cols, uint32_t new_pages)
	{
		if (new_cols == 0)
		{
			new_cols = cols_;
		}
		if (new_pages == 0)
		{
			new_pages = pages_;
		}
		//I have not yet implemented support for 2D or 3D bilerp
		LOGASSERT(new_cols == cols_ && new_pages == 1);
		ion::Matrix<T> result(new_rows, new_cols, new_pages);
		float row_scale = (float)rows_ / (float)new_rows;
		for (uint32_t row = 0; row < result.rows_; ++row)
		{
			float source_row = (float)row * row_scale;
			uint32_t input_1_row = (uint32_t)floorf(source_row);
			uint32_t input_2_row = ion::clamp((uint32_t)ceil(source_row), (uint32_t)0UL,(uint32_t)rows_-1);
			float distance = source_row - floorf(source_row);
			for (uint32_t col = 0; col < result.cols_; ++col)
			{
				//bilerp the two cells around the target cells
				result.At(row, col, 0) = static_cast<T>(static_cast<float>(At(input_1_row,col,0)) * (1.0f - distance) + static_cast<float>(At(input_2_row, col, 0)) * distance);
			}
		}
		return result;
	}
} //namespace ion
