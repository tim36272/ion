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
#include <limits>
#include "ionlib/queue.h"
#include "ionlib/net.h"
#ifdef OPENCV_CORE_HPP
#include "opencv2/imgproc.hpp"
#endif //#ifdef OPENCV_CORE_HPP
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
		//basic
		Matrix(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1, T* data = nullptr);
		Matrix(ion::TcpSocket& sock, bool receive_data);
		~Matrix();
		Matrix(const Matrix& rhs);
		Matrix operator=(const Matrix& rhs);
		void Resize(uint32_t rows, uint32_t cols = 1, uint32_t pages = 1);
		void DeepCopyTo(Matrix& rhs) const;
		Matrix DeepCopy() const;
		void Swap(Matrix& rhs);
		void SwapRoi(Matrix& rhs);
		template <class OtherType>
		void Cast(const Matrix<OtherType>& rhs);
		void Reshape(uint32_t new_rows, uint32_t new_cols = 1, uint32_t new_pages = 1);
		void Set(T val, uint32_t x, uint32_t y = 0, uint32_t z = 0);
		void SetAll(T val);
		T& At(uint32_t x, uint32_t y = 0, uint32_t z = 0);
		const T& At(uint32_t x, uint32_t y = 0, uint32_t z = 0) const;
		void Zero();
		void Eye();
		Matrix Diagonal();
		void Rand(double min, double max);
		Matrix Roi(uint32_t row_start, int64_t num_rows, uint32_t col_start = 0, int64_t num_cols = 0, uint32_t page_start = 0, int64_t num_pages = 0) const;
		void Roi_Fast(uint32_t row_start, int64_t num_rows, uint32_t col_start = 0, int64_t num_cols = 0, uint32_t page_start = 0, int64_t num_pages = 0, Matrix* reused_roi = NULL) const;
		void Roi_ReallyFast(uint32_t row_start, int64_t num_rows, uint32_t col_start = 0, int64_t num_cols = 0, uint32_t page_start = 0, int64_t num_pages = 0, Matrix* reused_roi = NULL) const;
		uint32_t rows() const;
		uint32_t cols() const;
		uint32_t pages() const;
		void Rowcat(const Matrix& rhs);
		void Colcat(const Matrix& rhs);
		void Pagecat(const Matrix& rhs);
		uint64_t NumDiff(const Matrix& rhs) const;
		size_t NumCells() const;
		bool IsContiguous() const;
		Matrix Map(const Matrix<uint32_t>& row_indices, const Matrix<uint32_t>& col_indices) const;
		void AssertNotNan() const;
		const Matrix View(uint32_t rows, uint32_t cols, uint32_t pages) const; //creates a view of the matrix as a different shape. I haven't thought this all the way though so it is const
		//serialization
		void PrintAscii(std::ostream& stream) const;
		void DumpBinary(std::ostream& stream) const;
		std::string Sprintf() const;
		enum PrintFmt
		{
			FMT_BIN,
			FMT_ASCII
		} ;
		void SetPrintFmt(PrintFmt format);
		PrintFmt GetPrintFmt() const;
		void Fread(FILE* fin, size_t num_elements);
		bool SendToSocket(ion::TcpSocket& sock) const;
		bool SendToSocketNoData(ion::TcpSocket& sock) const;
		bool RecvFromSocket(ion::TcpSocket& sock);
		//arithmetic
		void ElementwiseMultiply(const Matrix<T>& multiplier, Matrix<T>* result) const;
		void ElementwiseMultiplyRotated(const Matrix<T>& multiplier, Matrix<T>* result) const; //conceptually rotates the matrix 180 degrees before applying
		T ElementwiseMultiplyRotatedWithSumFast(const Matrix<T>& multiplier) const;
		T ElementwiseMultiplyRotatedWithSumFast2D(const Matrix<T>& multiplier) const;
		T ElementwiseMultiplyRotatedWithSumFastSSE3x3(const Matrix<T>& multiplier);
		T ElementwiseMultiplyRotatedWithSumFastSSE5x5(const Matrix<T>& multiplier);
		T Sum() const;
		ion::Matrix<T> Abs() const;
		Matrix SumRows() const;
		typedef T(*foreach_t)(T);
		typedef T(*foreachPair_t)(T, T);
		typedef bool(*foreachPairBool_t)(T, T);
		typedef T(*foreachPairUsrdata_t)(T, void*);
		//call function foreach on each element and store the result in result
		void Foreach(foreach_t foreach, Matrix<T>* result) const;
		//call function foreach on corresponding pairs of *this and rhs and store the result in result
		void Foreach(foreachPair_t foreach, const Matrix<T>& rhs, Matrix<T>* result) const;
		//call function foreach on each element along with constant and store the result in result
		void Foreach(foreachPair_t foreach, T constant, Matrix<T>* result) const;
		//call function foreach on each element along with constant (notice constant can be modified)
		void Foreach(foreachPair_t foreach, T* constant) const;
		//call function foreach on each element along with usrdata and store the result to result
		void Foreach(foreachPairUsrdata_t foreach, void* usrdata, Matrix<T>*result) const;
		template <class U>
		friend Matrix<U> operator+(const Matrix<U>& lhs, const Matrix<U>& rhs);
		template <class U>
		friend Matrix<U> operator+(const Matrix<U>& lhs, U rhs);
		template <class U>
		friend Matrix<U> operator-(const Matrix<U>& lhs, const Matrix<U>& rhs);
		template <class U>
		friend Matrix<U> operator-(U rhs, const Matrix<U>& lhs);
		template <class U>
		friend Matrix<U> operator*(const Matrix<U>& lhs, const Matrix<U>& rhs);
		template <class U>
		friend Matrix<U> operator*(const Matrix<U>& lhs, U rhs);
		template <class U>
		friend Matrix<U> operator/(const Matrix<U>& lhs, U rhs);
		template <class U>
		friend Matrix<U> operator/(U lhs, const Matrix<U>& rhs);
		T Dot(const Matrix<T>& rhs) const;
		Matrix DotAsColumns(const Matrix<T>& rhs) const;
		void Transpose(Matrix<T>* result);
		void Inverse(Matrix<T>* result);
		T Determinent() const;
		T Max() const;
		T Mean() const;
		Matrix Log() const;
		uint64_t Argmax() const; //computes argmax by flattening the array first
		ion::Matrix<uint32_t> Argmax(uint32_t dim) const;
		uint64_t Countif(foreachPairBool_t foreach, T constant) const;
		//filtering
		enum class ConvFlag
		{
			CONV_FLAG_ZERO_PAD = 1,
			CONV_FLAG_MIRROR = 2,
			CONV_FLAG_COPY = 4,
			CONV_FLAG_WRAPAROND = 8,
			CONV_FLAG_NO_PAD = 16,
			CONV_FLAG_SPARSE_Z = 32
		};
		template <class U>
		friend ion::Matrix<U> Convolve(const ion::Matrix<U>& mat, const ion::Matrix<U>& kernel, typename ion::Matrix<U>::ConvFlag flags);
		template <class U>
		friend ion::Matrix<U> ConvolveDryRun(const ion::Matrix<U>& mat, const ion::Matrix<U>& kernel, typename ion::Matrix<U>::ConvFlag flags); //just returns a matrix the right shape
		template <class U>
		friend bool indexInPad(const Matrix<U>& mat_padded, const Matrix<U>& kernel, uint32_t row, uint32_t col, uint32_t page);
		template <class U>
		friend ion::Matrix<U> MaxPool(const ion::Matrix<U>& mat, uint32_t pool_size);
		template <class U>
		friend ion::Matrix<U> Softmax(const ion::Matrix<U>& mat);
		Matrix ResampleBilerp(uint32_t new_rows, uint32_t new_cols = 0, uint32_t new_pages = 0);

		uint64_t user_id; //user can use this for whatever they want
#ifdef OPENCV_CORE_HPP
		//OpenCV
		void imshow();
		cv::Mat asCvMat();
#endif
	private:
		void Construct(uint32_t rows, uint32_t cols, uint32_t pages, T* data);
		void Destruct();
		Matrix() = delete; //default construction is only allowed by the library so this is private
		//size of the non-roi matrix. It is guaranteed that (data_ + rows_*cols_*pages_) is the last element if !is_roi_
		uint32_t rows_;
		uint32_t cols_;
		uint32_t pages_;
		size_t   allocated_cells_;
		//size of the roi matrix, if any. It is guaranteed that roi_rows_ <= rows_, etc.
		uint32_t roi_row_origin_;
		uint32_t roi_col_origin_;
		uint32_t roi_page_origin_;
		//these differ from rows_,cols_, and pages_ because it specifies the stride between rols/cols/pages (that is, for an ROI, this is >= rows_ cols_ pages_)
		//Note that it is NOT necessary that allocated_rows_*allocated_cols_*allocated_pages_ == allocated_cells_ (if not all the allocated data is being used)
		uint32_t allocated_rows_;
		uint32_t allocated_cols_;
		uint32_t allocated_pages_;
		bool     contiguous_;
		bool     is_roi_;
		T*       data_;
		PrintFmt format_;
		bool     data_is_caller_provided_; //data was not malloc'd by this library
	};
	template <class T>
	std::ostream& operator<< (std::ostream& out, const ion::Matrix<T>& mat);
	template <class T>
	ion::Matrix<T> Linspace(T min, T max);
	
#define CONVOLVE_TASK_MAGIC       (0x0123456789ABCDEF)
#define CONVOLVE_TASK_REPLY_MAGIC (0xFEDCBA9876543210)
	typedef float neuronWorker_t;
	template <class T>
	struct ConvolveTask
	{
		static const uint64_t magic = CONVOLVE_TASK_MAGIC;
		//The operation to be performed is: for each image: for each kernel: convolve image with kernel and store the result to result[image_index][kernel_index]
		uint32_t num_inputs;
		uint32_t num_kernels;
		std::vector<ion::Matrix<T>>* input; //an array of num_input matrices
		std::vector<ion::Matrix<T>>* kernel;//an array of num_kernels matrices
		std::vector<std::vector<ion::Matrix<T>>>* result; //an array of num_inputs x num_kernels matrices 
		typename ion::Matrix<T>::ConvFlag flags; //flags to use during the convolution
	};
	template <class T>
	struct ConvolutionTaskerData
	{
		ConvolutionTaskerData()
		{
		}
		ion::Queue<ConvolveTask<T>> task_queue;
		ion::Queue<ConvolveTask<T>> result_queue;
		uint16_t announcement_port; //network byte order
	};
	//This will spawn num_threads threads which will wait on convolution operations in task_queue and process them
	template <class T>
	void InitConvolutionTasker(uint32_t num_threads, ConvolutionTaskerData<T>& task_data);

	template <class T>
	void PushConvolutionTask(ConvolutionTaskerData<T>& tasker, ConvolveTask<T>& task);

	template <class T>
	void JoinConvolveTasks(ConvolutionTaskerData<T>& tasker);

	struct ConvolveWorkerAnnouncement_t
	{
		uint16_t recv_port; //network byte order
	};
	struct ConvolveWorker_t
	{
		uint16_t work_recv_port; //receives work on this port
		ion::IpAddress addr; //used to disambiguate workers
		ion::TcpSocket sock;
		ConvolveTask<neuronWorker_t> current_task;
	};
	ion::Error SendConvolutionWorkerAnnouncement(ion::UdpSocket socket, ion::IpAddress addr, uint16_t port, ConvolveWorkerAnnouncement_t announcement, ConvolveWorker_t& worker);
	ion::Error GetConvolutionTask(ConvolveWorker_t& worker, ConvolveTask<neuronWorker_t>& task);
	ion::Error SendConvolutionTaskResult(ConvolveWorker_t& worker);

	uint64_t GetMatrixAllocations();
	uint64_t GetMatrixDeletions();
}; //namespace ion

#endif //ION_MATRIX_H_
