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
	static uint64_t allocations_g = 0;
	static uint64_t deletions_g = 0;
	uint64_t GetMatrixAllocations()
	{
		LOGWEAKASSERT(allocations_g >= deletions_g, "Allocations: %llu, Deletions: %llu", allocations_g, deletions_g);
		return allocations_g;
	}
	uint64_t GetMatrixDeletions()
	{
		LOGWEAKASSERT(allocations_g >= deletions_g, "Allocations: %llu, Deletions: %llu", allocations_g, deletions_g);
		return deletions_g;
	}

	//////////////////////////////////////////////////// Constructors
	//template <class T>
	//ion::Matrix<T>::Matrix()
	//{
	//	LOGASSERT(allocations_g >= deletions_g);

	//	//consider checking here if rows*cols*pages fits in a 64 bit integer
	//	allocated_cells_ = 0;
	//	data_ = nullptr;
	//	rows_ = 0;
	//	cols_ = 0;
	//	pages_ = 0;
	//	allocated_rows_ = 0;
	//	allocated_cols_ = 0;
	//	allocated_pages_ = 0;
	//	roi_row_origin_ = 0;
	//	roi_col_origin_ = 0;
	//	roi_page_origin_ = 0;
	//	contiguous_ = true;
	//	is_roi_ = false;
	//	format_ = FMT_ASCII;
	//}
	template <class T>
	void ion::Matrix<T>::Construct(uint32_t rows, uint32_t cols, uint32_t pages)
	{

		//consider checking here if rows*cols*pages fits in a 64 bit integer
		allocated_cells_ = (uint64_t)rows * (uint64_t)cols * (uint64_t)pages;
		//allocate the matrix
		if (allocated_cells_ > 0)
		{
			data_ = new T[allocated_cells_];
			allocations_g++;
			LOGASSERT(data_ != NULL);
		} else
		{
			data_ = nullptr;
		}
		rows_ = rows;
		cols_ = cols;
		pages_ = pages;
		allocated_rows_ = rows;
		allocated_cols_ = cols;
		allocated_pages_ = pages;
		roi_row_origin_ = 0;
		roi_col_origin_ = 0;
		roi_page_origin_ = 0;
		contiguous_ = true;
		is_roi_ = false;
		format_ = FMT_ASCII;
		user_id = 0ULL;
		LOGWEAKASSERT(allocations_g >= deletions_g, "Allocations: %llu, Deletions: %llu", allocations_g, deletions_g);
	}
	template <class T>
	ion::Matrix<T>::Matrix(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1)
	{
		Construct(rows, cols, pages);
	}
	template <class T>
	ion::Matrix<T>::~Matrix()
	{

		if (!is_roi_)
		{
			delete[] data_;
			deletions_g++;
		}
		LOGWEAKASSERT(allocations_g >= deletions_g, "Allocations: %llu, Deletions: %llu", allocations_g, deletions_g);
	}
	template <class T>
	ion::Matrix<T>::Matrix(const ion::Matrix<T>& rhs)
	{

		LOGASSERT(this != &rhs);
		allocated_cells_ = rhs.allocated_cells_;
		if (!rhs.is_roi_)
		{
			data_ = new T[rhs.allocated_cells_];
			allocations_g++;
			//copy the data
			memcpy(data_, rhs.data_, allocated_cells_ * sizeof(T));
		} else
		{
			data_ = rhs.data_;
		}
		LOGASSERT(data_ != NULL);
		rows_ = rhs.rows_;
		cols_ = rhs.cols_;
		pages_ = rhs.pages_;
		allocated_rows_ = rhs.allocated_rows_;
		allocated_cols_ = rhs.allocated_cols_;
		allocated_pages_ = rhs.allocated_pages_;
		roi_row_origin_ = rhs.roi_row_origin_;
		roi_col_origin_ = rhs.roi_col_origin_;
		roi_page_origin_ = rhs.roi_page_origin_;
		contiguous_ = rhs.contiguous_;
		is_roi_ = rhs.is_roi_;
		format_ = rhs.format_;
		user_id = rhs.user_id;
		LOGWEAKASSERT(allocations_g >= deletions_g, "Allocations: %llu, Deletions: %llu", allocations_g, deletions_g);
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::operator=(const ion::Matrix<T>& rhs)
	{

		LOGASSERT(this != &rhs);
		//delete *this
		if (!is_roi_)
		{
			delete data_;
			deletions_g++;
		}
		allocated_cells_ = rhs.allocated_cells_;
		if (!rhs.is_roi_)
		{
			data_ = new T[rhs.allocated_cells_];
			allocations_g++;

			//copy the data
			memcpy(data_, rhs.data_, allocated_cells_ * sizeof(T));
		} else
		{
			data_ = rhs.data_;
		}
		LOGASSERT(data_ != NULL);
		rows_ = rhs.rows_;
		cols_ = rhs.cols_;
		pages_ = rhs.pages_;
		allocated_rows_ = rhs.allocated_rows_;
		allocated_cols_ = rhs.allocated_cols_;
		allocated_pages_ = rhs.allocated_pages_;
		roi_row_origin_ = rhs.roi_row_origin_;
		roi_col_origin_ = rhs.roi_col_origin_;
		roi_page_origin_ = rhs.roi_page_origin_;
		contiguous_ = rhs.contiguous_;
		is_roi_ = rhs.is_roi_;
		format_ = rhs.format_;
		user_id = rhs.user_id;
		LOGWEAKASSERT(allocations_g >= deletions_g, "Allocations: %llu, Deletions: %llu", allocations_g, deletions_g);
		return *this;
	}
	//////////////////////////////////////////////////// Elementry Operations
	template <class T>
	void ion::Matrix<T>::Resize(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1)
	{
		//it doesn't make sense to resize an ROI (just get a new one)
		LOGASSERT(!is_roi_);
		uint64_t new_size = (uint64_t)rows * (uint64_t)cols * (uint64_t)pages;
		if (new_size > allocated_cells_)
		{
			//LOGWARN("The requested matrix size %llu is greater than the allocated size %llu. The matrix will be re-allocated but this can lead to severe performance penalties", new_size, allocated_cells_);
			T* old_data = data_;
			data_ = new T[new_size];
			LOGASSERT(data_ != NULL);
			memcpy(data_, old_data, allocated_cells_);
			allocated_cells_ = new_size;
			delete old_data;
			contiguous_ = true;
			allocated_rows_ = rows;
			allocated_cols_ = cols;
			allocated_pages_ = pages;
		}
		rows_ = rows;
		cols_ = cols;
		pages_ = pages;
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::DeepCopy() const
	{
		ion::Matrix<T> rhs(rows_, cols_, pages_);
		rhs.user_id = user_id;
		//this function is ROI-safe
		//if the input is an ROI, the result will be deep coppied into the parent's data (i.e. it is still an ROI)
		if (contiguous_ && rhs.contiguous_)
		{
			//Note it is NOT necessary that rows_*cols_*pages_ == allocated_cells_
			//The MAT_INDEX is added in case roi_row_origin_ != 0
			memcpy(rhs.data_ + MAT_INDEX(rhs, 0, 0, 0), data_ + MAT_INDEX(*this, 0, 0, 0), rows_*cols_*pages_ * sizeof(T));
			return rhs;
		} else
		{
			//do it the hard way. We still might be able to optimize part of it, though
			//Notice that, even though it would work, I don't allow it to do this if pages_==1 because it would be more efficient to just assign everything
			if (pages_ != 1 && ((pages_ == allocated_pages_) && (rhs.pages_ == rhs.allocated_pages_)))
			{
				//this means we can copy each *column* via memcpy
				//it is implicit that we can't copy each row otherwise contiguous_ would have been set
				for (uint32_t row_index = 0; row_index < rows_; ++row_index)
				{
					for (uint32_t col_index = 0; col_index < cols_; ++col_index)
					{
						//compute where this column starts at
						size_t col_start = MAT_INDEX(*this, row_index, col_index, 0);
						size_t rhs_col_start = MAT_INDEX(rhs, row_index, col_index, 0);
						memcpy(rhs.data_ + rhs_col_start, data_ + col_start, pages_ * sizeof(T));
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
							rhs.At(row_index, col_index, page_index) = At(row_index, col_index, page_index);
						}
					}
				}
			}
		}
		return rhs;
	}
	template <class T>
	void ion::Matrix<T>::DeepCopyTo(Matrix& rhs) const
	{
		LOGASSERT(rows_ == rhs.rows_);
		LOGASSERT(cols_ == rhs.cols_);
		LOGASSERT(pages_ == rhs.pages_);
		rhs.user_id = user_id;
		//this function is ROI-safe
		//if the input is an ROI, the result will be deep coppied into the parent's data (i.e. it is still an ROI)
		if (contiguous_ && rhs.contiguous_)
		{
			//Note it is NOT necessary that rows_*cols_*pages_ == allocated_cells_
			//The MAT_INDEX is added in case roi_row_origin_ != 0
			memcpy(rhs.data_ + MAT_INDEX(rhs, 0, 0, 0), data_ + MAT_INDEX(*this, 0, 0, 0), rows_*cols_*pages_ * sizeof(T));
			return;
		} else
		{
			//do it the hard way. We still might be able to optimize part of it, though
			//Notice that, even though it would work, I don't allow it to do this if pages_==1 because it would be more efficient to just assign everything
			if (pages_ != 1 && ((pages_ == allocated_pages_) && (rhs.pages_ == rhs.allocated_pages_)))
			{
				//this means we can copy each *column* via memcpy
				//it is implicit that we can't copy each row otherwise contiguous_ would have been set
				for (uint32_t row_index = 0; row_index < rows_; ++row_index)
				{
					for (uint32_t col_index = 0; col_index < cols_; ++col_index)
					{
						//compute where this column starts at
						size_t col_start = MAT_INDEX(*this, row_index, col_index, 0);
						size_t rhs_col_start = MAT_INDEX(rhs, row_index, col_index, 0);
						memcpy(rhs.data_ + rhs_col_start, data_ + col_start, pages_ * sizeof(T));
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
							rhs.At(row_index, col_index, page_index) = At(row_index, col_index, page_index);
						}
					}
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Swap(Matrix& rhs)
	{
		//see SwapRoi for ROI-enabled version
		LOGASSERT(!is_roi_);
		LOGASSERT(!rhs.is_roi_);
		//make a shallow copy and use that to swap
		Matrix temp = rhs;
		rhs = *this;
		*this = temp;
	}
	template <class T>
	void ion::Matrix<T>::SwapRoi(Matrix& rhs)
	{
		LOGASSERT(rows_ == rhs.rows_ && cols_ == rhs.cols_ && pages_ == rhs.pages_);
		for (uint32_t row_index = 0; row_index < rows_; ++row_index)
		{
			for (uint32_t col_index = 0; col_index < cols_; ++col_index)
			{
				for (uint32_t page_index = 0; page_index < pages_; ++page_index)
				{
					std::swap(At(row_index, col_index, page_index),rhs.At(row_index, col_index, page_index));
				}
			}
		}
	}
	template <class T>
	template <class OtherType>
	void ion::Matrix<T>::Cast(const Matrix<OtherType>& rhs)
	{
		//this function is ROI-safe
		//Notice that since rhs is not the same type as *this I don't have access to rhs's private data
		LOGASSERT(rhs.rows() == rows_);
		LOGASSERT(rhs.cols() == cols_);
		LOGASSERT(rhs.pages() == pages_);

		for (uint32_t row_index = 0; row_index < rows_; ++row_index)
		{
			for (uint32_t col_index = 0; col_index < cols_; ++col_index)
			{
				for (uint32_t page_index = 0; page_index < pages_; ++page_index)
				{
					At(row_index, col_index, page_index) = (T)rhs.At(row_index, col_index, page_index);
				}
			}
		}
	}
	void InferDimensions(uint64_t size, uint32_t& rows, uint32_t& cols, uint32_t& pages)
	{
		if (rows == 0)
		{
			if (cols == 0 || pages == 0)
			{
				LOGFATAL("Invalid attempt to infer parameters")
			}
			rows = (uint32_t)(size / cols / pages);
		} else if (cols == 0)
		{
			if (rows == 0 || pages == 0)
			{
				LOGFATAL("Invalid attempt to infer parameters")
			}
			cols = (uint32_t)(size / rows / pages);
		} else if (pages == 0)
		{
			if (rows == 0 || cols == 0)
			{
				LOGFATAL("Invalid attempt to infer parameters")
			}
			pages = (uint32_t)(size / rows / cols);
		}
	}
	template <class T>
	void ion::Matrix<T>::Reshape(uint32_t new_rows, uint32_t new_cols = 1, uint32_t new_pages = 1)
	{
		//note that old_size is not necessarily equal to allocated_cells_ because the matrix could be a 3x3 allocated in 4x4 cells, for example
		uint64_t old_size = (uint64_t)rows_ * (uint64_t)cols_* (uint64_t)pages_;
		InferDimensions(old_size, new_rows, new_cols, new_pages);
		//you cannot reshape an ROI since it doesn't own the data. Deep copy it first.
		LOGASSERT(!is_roi_);
		//this function maintains the data, whereas Resize doesn't have to. For example if *this is a row vector and you resize it to a column vector it will work, whereas Resize may not
		uint64_t new_size = (uint64_t)new_rows * (uint64_t)new_cols * (uint64_t)new_pages;
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
					uint32_t src_col = (uint32_t)((src_offset / old_mat.rows_) % old_mat.cols_);
					uint32_t src_page = (uint32_t)((src_offset / (old_mat.rows_*old_mat.cols_)) % old_mat.pages_);
					At(row_index, col_index, page_index) = old_mat.At(src_row, src_col, src_page);
				}
			}
		}
		//old_mat will automatically be deleted by the destructor
	}
	template <class T>
	void ion::Matrix<T>::SetAll(T val)
	{
		//this function is ROI-safe
		for (uint32_t row_index = 0; row_index < rows_; ++row_index)
		{
			for (uint32_t col_index = 0; col_index < cols_; ++col_index)
			{
				for (uint32_t page_index = 0; page_index < pages_; ++page_index)
				{
					At(row_index, col_index, page_index) = val;
				}
			}
		}
	}
	template <class T>
	T& ion::Matrix<T>::At(uint32_t x, uint32_t y = 0, uint32_t z = 0)
	{
		//this function is ROI-safe
		return data_[MAT_INDEX(*this, x, y, z)];
	}
	template <class T>
	const T& ion::Matrix<T>::At(uint32_t x, uint32_t y = 0, uint32_t z = 0) const
	{
		//Since the const version of this function is being called it must be that we are going to read that value, not set it. So do a sanity check that the values is good
		LOGSANITY(!isnan((double)data_[MAT_INDEX(*this, x, y, z)]));
		//this function is ROI-safe
		return data_[MAT_INDEX(*this, x, y, z)];
	}
	template <class T>
	void ion::Matrix<T>::Zero()
	{
		//this function is ROI-safe
		//check if the mat is contiguous. If so it can simply be memset
		if (contiguous_)
		{
			//note that you don't want to memset allocated_cells_ because the data could be contiguous but not take up the whole allocated space
			//The MAT_INDEX is added in case roi_row_origin_ != 0
			memset(data_ + MAT_INDEX(*this, 0, 0, 0), 0, rows_ * cols_ * pages_ * sizeof(*data_));
		} else
		{
			//do it the hard way
			for (uint32_t row_index = 0; row_index < rows_; ++row_index)
			{
				for (uint32_t col_index = 0; col_index < cols_; ++col_index)
				{
					for (uint32_t page_index = 0; page_index < pages_; ++page_index)
					{
						At(row_index, col_index, page_index) = static_cast<T>(0);
					}
				}
			}
		}
	}
	template <class T>
	void ion::Matrix<T>::Eye()
	{
		//this function is ROI-safe
		//this can only be run on square matrices
		LOGASSERT(rows_ == cols_ && (pages_ == 1));
		Zero();
		for (uint32_t row_index = 0; row_index < rows_; ++row_index)
		{
			At(row_index, row_index, 1) = static_cast <T>(1);
		}
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::Diagonal()
	{
		//cases:
		//	(1,1,1) Legal	(1,1)(2,1)(3,1)
		//	(1,2,1) Illegal (2)
		//	(1,1,2) Illegal (3)
		//	(1,2,2) Legal	(1,1)(2,3)(3,3)
		//	(1,2,3) Illegal (2)

		//	(2,1,1) Illegal (1)
		//	(2,2,1) Legal	(1,2)(2,2)(3,1)
		//	(2,1,2) Legal	(1,3)(2,1)(3,2)
		//	(2,2,2) Legal	(1,2)(2,2)(3,2)
		//	(2,2,3) Illegal (3)

		//	(3,2,1) Illegal (1)
		//	(3,1,2) Illegal (1)
		//	(3,2,2) Illegal (1)
		//	(3,2,3) Illegal (2)
		LOGASSERT((rows_  == 1) || (rows_  == cols_) || (rows_ == pages_));
		LOGASSERT((cols_  == 1) || (cols_  == rows_) || (cols_ == pages_));
		LOGASSERT((pages_ == 1) || (pages_ == rows_) || (pages_ == cols_));

		uint32_t row_step  = rows_ == 1 ? 0 : 1;
		uint32_t col_step  = cols_ == 1 ? 0 : 1;
		uint32_t page_step = pages_ == 1 ? 0 : 1;
		uint32_t row_index = 0, col_index = 0, page_index = 0;
		uint32_t num_elements = ion::Max(ion::Max(rows_, cols_), pages_);

		ion::Matrix<T> result(num_elements);
		for (uint32_t element_index = 0; element_index < num_elements; ++element_index)
		{
			result.At(element_index) = At(row_index, col_index, page_index);
			row_index += row_step;
			col_index += col_step;
			page_index += page_step;
		}
		return result;
	}
	template <class T>
	void ion::Matrix<T>::Rand(double min, double max)
	{
		//this function is inplace safe
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					At(row, col, page) = static_cast<T>(randlf(min, max));
				}
			}
		}
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::Roi(uint32_t row_start, int64_t num_rows, uint32_t col_start, int64_t num_cols, uint32_t page_start, int64_t num_pages) const
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
		Matrix result(0,0,0);

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
		result.contiguous_ = IsContiguous();
		result.user_id = user_id;
		return result;
	}
	template <class T>
	void ion::Matrix<T>::Roi_Fast(uint32_t row_start, int64_t num_rows, uint32_t col_start, int64_t num_cols, uint32_t page_start, int64_t num_pages, ion::Matrix<T>* reused_roi) const
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

		reused_roi->is_roi_ = true;
		reused_roi->data_ = data_;
		reused_roi->allocated_cells_ = 0;
		reused_roi->rows_ = (uint32_t)num_rows;
		reused_roi->cols_ = (uint32_t)num_cols;
		reused_roi->pages_ = (uint32_t)num_pages;
		reused_roi->roi_row_origin_ = row_start + roi_row_origin_;
		reused_roi->roi_col_origin_ = col_start + roi_col_origin_;
		reused_roi->roi_page_origin_ = page_start + roi_page_origin_;
		reused_roi->allocated_rows_ = allocated_rows_;
		reused_roi->allocated_cols_ = allocated_cols_;
		reused_roi->allocated_pages_ = allocated_pages_;
		reused_roi->format_ = format_;
		reused_roi->contiguous_ = IsContiguous();
		reused_roi->user_id = user_id;
	}
	template <class T>
	uint32_t ion::Matrix<T>::rows() const
	{
		return rows_;
	}
	template <class T>
	uint32_t ion::Matrix<T>::cols() const
	{
		return cols_;
	}
	template <class T>
	uint32_t ion::Matrix<T>::pages() const
	{
		return pages_;
	}
	template <class T>
	void ion::Matrix<T>::Set(T val, uint32_t x, uint32_t y = 0, uint32_t z = 0)
	{
		//this function is ROI-safe
		At(x, y, z) = val;
	}
	template <class T>
	void ion::Matrix<T>::Rowcat(const Matrix<T>& rhs)
	{
		//verify there is enough space
		LOGASSERT(allocated_rows_ >= roi_row_origin_ + rows_ + rhs.rows_);
		LOGASSERT(cols_ == rhs.cols_ && pages_ == rhs.pages_);
		for (uint32_t row = 0; row < rhs.rows_; ++row)
		{
			for (uint32_t col = 0; col < rhs.cols_; ++col)
			{
				for (uint32_t page = 0; page < rhs.pages_; ++page)
				{
					this->At(rows_ + row, col, page) = rhs.At(row, col, page);
				}
			}
		}
		rows_ += rhs.rows_;
		//This has potential to make the matrix no longer contiguous
		contiguous_ = IsContiguous();
	}
	template <class T>
	void ion::Matrix<T>::Colcat(const Matrix<T>& rhs)
	{
		//verify there is enough space
		LOGASSERT(allocated_cols_ >= roi_col_origin_ + cols_ + rhs.cols_);
		LOGASSERT(rows_ == rhs.rows_ && pages_ == rhs.pages_);
		for (uint32_t row = 0; row < rhs.rows_; ++row)
		{
			for (uint32_t col = 0; col < rhs.cols_; ++col)
			{
				for (uint32_t page = 0; page < rhs.pages_; ++page)
				{
					this->At(row, cols_ + col, page) = rhs.At(row, col, page);
				}
			}
		}
		cols_ += rhs.cols_;
		//This has potential to make the matrix no longer contiguous
		contiguous_ = IsContiguous();
	}
	template <class T>
	void ion::Matrix<T>::Pagecat(const Matrix<T>& rhs)
	{
		//verify there is enough space
		LOGASSERT(allocated_pages_ >= roi_page_origin_ + pages_ + rhs.pages_);
		LOGASSERT(rows_ == rhs.rows_ && cols_ == rhs.cols_);
		for (uint32_t row = 0; row < rhs.rows_; ++row)
		{
			for (uint32_t col = 0; col < rhs.cols_; ++col)
			{
				for (uint32_t page = 0; page < rhs.pages_; ++page)
				{
					this->At(row, col, pages_ + page) = rhs.At(row, col, page);
				}
			}
		}
		pages_ += rhs.pages_;
		//This has potential to make the matrix no longer contiguous
		contiguous_ = IsContiguous();
	}
	template <class T>
	uint64_t ion::Matrix<T>::NumDiff(const ion::Matrix<T>& rhs) const
	{
		LOGASSERT(rows_ == rhs.rows_ && cols_ == rhs.cols_ && pages_ == rhs.pages_);
		uint64_t result = 0;
		for (uint32_t row = 0; row < rhs.rows_; ++row)
		{
			for (uint32_t col = 0; col < rhs.cols_; ++col)
			{
				for (uint32_t page = 0; page < rhs.pages_; ++page)
				{
					if (At(row, col, page) != rhs.At(row, col, page))
					{
						result++;
					}
				}
			}
		}
		return result;
	}
	template <class T>
	uint64_t ion::Matrix<T>::NumCells() const
	{
		return rows_ * cols_ * pages_;
	}
	template <class T>
	bool ion::Matrix<T>::IsContiguous() const
	{
		//note it is not required that row_start == 0 nor that rows_ == num_rows because the data can still be contiguous in the middle of the allocated region
		if (roi_col_origin_ == 0 && roi_page_origin_ == 0 && allocated_cols_ == cols_ && allocated_pages_ == pages_)
		{
			return true;
		} else
		{
			return false;
		}
	}
	template <class T>
	ion::Matrix<T> ion::Matrix<T>::Map(const Matrix<uint32_t>& row_indices, const Matrix<uint32_t>&col_indices) const
	{
		//I haven't implemetned this for pages yet, but I could
		LOGASSERT(pages_ == 1);
		LOGASSERT(row_indices.rows() == col_indices.rows());
		LOGASSERT(row_indices.cols() == 1 && row_indices.pages() == 1);
		LOGASSERT(col_indices.cols() == 1 && col_indices.pages() == 1);
		ion::Matrix<T> result(row_indices.rows());
		result.user_id = user_id;
		for (uint32_t index = 0; index < result.rows_; ++index)
		{
			uint32_t row_index = row_indices.At(index);
			uint32_t col_index = col_indices.At(index);
			LOGASSERT(row_index < rows_);
			LOGASSERT(col_index < cols_);
			result.At(index) = At(row_index,col_index);
		}
		return result;
	}
	template <class T>
	void ion::Matrix<T>::AssertNotNan() const
	{
		for (uint32_t row = 0; row < rows_; ++row)
		{
			for (uint32_t col = 0; col < cols_; ++col)
			{
				for (uint32_t page = 0; page < pages_; ++page)
				{
					LOGASSERT(!isnan((double)At(row, col, page)), "(%u,%u,%u)",row,col,page);
				}
			}
		}
	}
	template <class T>
	const ion::Matrix<T> ion::Matrix<T>::View(uint32_t rows, uint32_t cols, uint32_t pages) const
	{
		uint64_t old_size = (uint64_t)rows_ * (uint64_t)cols_* (uint64_t)pages_;
		InferDimensions(old_size, rows, cols, pages);
		LOGASSERT(contiguous_ && rows_ * cols_ * pages_ == rows*cols*pages);
		Matrix<T> result = Roi(0, 0, 0, 0, 0, 0);
		result.rows_ = rows;
		result.cols_ = cols;
		result.pages_ = pages;
		return result;
	}
	template <class T>
	ion::Matrix<T> Linspace(T min, T max)
	{
		ion::Matrix<T> result((uint32_t)(max - min));
		uint32_t row_idx = 0;
		for (T idx = min; idx < max; ++idx)
		{
			result.At(row_idx) = idx;
			++row_idx;
		}
		return result;
	}


} //namespace ion
