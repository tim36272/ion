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
#include "ionlib/thread.h"
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
								uint32_t src_row = kernel.rows_ / 2 + MAX((int64_t)row - 1, 0);
								uint32_t src_col = kernel.cols_ / 2 + MAX((int64_t)col - 1, 0);
								uint32_t src_page = kernel.pages_ / 2 + MAX((int64_t)page - 1, 0);
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
		Matrix<T> temp(kernel.rows_, kernel.cols_, kernel.pages_);
		Matrix<T> src_roi = mat_padded.Roi(0, 0);
#pragma loop(hint_parallel(8))
		for (uint32_t row = 0; row < output.rows_; ++row)
		{
			for (uint32_t col = 0; col < output.cols_; ++col)
			{
				if ((uint32_t)flags & (uint32_t)Matrix<T>::ConvFlag::CONV_FLAG_SPARSE_Z)
				{
					//check if this page is 0
					bool all_zero = true;
					for (uint32_t page = 0; page < output.pages_; ++page)
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
				for (uint32_t page = 0; page < output.pages_; ++page)
				{
					//now row,col,page is centered around the cell to convolve
					//make a region of interest around this cell
					mat_padded.Roi_Fast(row, kernel.rows_, col, kernel.cols_, page, kernel.pages_, &src_roi);
					src_roi.ElementwiseMultiplyRotated(kernel, &temp);
					//this doesn't use At for performance reasons
					output.data_[MAT_INDEX(output,row, col, page)] = temp.Sum();
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
		uint32_t row_step = (mat.rows_ == 1) ? (1) : (pool_size);
		uint32_t col_step = (mat.cols_ == 1) ? (1) : (pool_size);
		uint32_t page_step = (mat.pages_ == 1) ? (1) : (pool_size);
		ion::Matrix<T> result(result_rows, result_cols, result_pages);
		for (uint32_t row = 0; row < mat.rows_; row += row_step)
		{
			for (uint32_t col = 0; col < mat.cols_; col += col_step)
			{
				for (uint32_t page = 0; page < mat.pages_; page += page_step)
				{
					//create ROI for this macrovoxel
					ion::Matrix<T> roi = mat.Roi(row, row_step, col, col_step, page, page_step);
					result.At(row / pool_size, col / pool_size, page / pool_size) = roi.Max();
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
			mat.Roi(row,1).Foreach(&ion::Exp, &temp);
			T sum = temp.Sum();
			LOGASSERT(sum > static_cast<T>(0));
			//Calculate softmax
			ion::Matrix<T> result_roi = result.Roi(row, 1);
			temp.Foreach(&ion::Divide, sum, &result_roi);
		}
		return result;
	}
	
	template <class T>
	void ConvolutionThread(void* usrdata)
	{
		ConvolveTaskData<T>* task_data = (ConvolveTaskData<T>*)usrdata;
		ConvolveTask<T> task;
		while (true)
		{
			//get an element from the queue and do the work
			(void)task_data->task_queue.Pop(0, &task);
			*(task.result) = ion::Convolve(*(task.input), *(task.kernel), task.flags);
			task_data->result_queue.Push(task);
		}
	}

	template <class T>
	void InitConvolveThreads(uint32_t num_threads, ConvolveTaskData<T>& task_data)
	{
		for (uint32_t thread_index = 0; thread_index < num_threads; ++thread_index)
		{
			ion::StartThread(ConvolutionThread<double>, &task_data);
		}
	}


} //namespace ion
