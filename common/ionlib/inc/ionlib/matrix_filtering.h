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
#include "ionlib/net.h"
#include "ionlib/time.h"
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
		float row_scale = (float)rows_ / new_rows;
		for (uint32_t row = 0; row < result.rows_; ++row)
		{
			float source_row = row * row_scale;
			uint32_t input_1_row = (uint32_t)floor(source_row);
			uint32_t input_2_row = ion::clamp((uint32_t)ceil(source_row), (uint32_t)0UL,(uint32_t)rows_-1);
			float distance = source_row - floor(source_row);
			for (uint32_t col = 0; col < result.cols_; ++col)
			{
				//bilerp the two cells around the target cells
				result.At(row, col, 0) = static_cast<T>(At(input_1_row,col,0) * (1.0f - distance) + At(input_2_row, col, 0) * (distance));
			}
		}
		return result;
	}
	//template <class T>
	//void ConvolutionThread(void* usrdata)
	//{
	//	ConvolutionTasker<T>* task_data = (ConvolutionTasker<T>*)usrdata;
	//	ConvolveTask<T> task;
	//	while (true)
	//	{
	//		//get an element from the queue and do the work
	//		(void)task_data->task_queue.Pop(0, &task);
	//		*(task.result) = ion::Convolve(*(task.input), *(task.kernel), task.flags);
	//		task_data->result_queue.Push(task);
	//	}
	//}

#define MAX_CONVOLVE_WORKERS 256
#define INVALID_CONVOLVE_WORKER 0xFFFFFFFF
	uint32_t WorkerInPool(std::vector<ConvolveWorker_t>& pool, ConvolveWorker_t worker)
	{
		for (std::vector<ConvolveWorker_t>::iterator pool_it = pool.begin(); pool_it != pool.end();++pool_it)
		{
			if (worker.addr.as_integer() == pool_it->addr.as_integer() && worker.work_recv_port == pool_it->work_recv_port)
			{
				return (uint32_t)(pool_it-pool.begin());
			}
		}
		return INVALID_CONVOLVE_WORKER;
	}
	bool InitWorker(ConvolveWorker_t& worker)
	{
		ion::Error result;
		worker.sock.Create(worker.work_recv_port, worker.addr);
		result = worker.sock.Listen();
		if (!result.success())
		{
			LOGWARN("Failed to listen socket for worker at IP %s port %d", worker.addr.as_string(), worker.work_recv_port.AsHostOrder());
			return false;
		}
		return true;
	}
	void InsertWorker(std::vector<ConvolveWorker_t>& pool, ConvolveWorker_t worker)
	{
		pool.push_back(worker);
	}
	bool SendWorkerTask(ConvolveWorker_t& worker, ConvolveTask<neuronWorker_t>& task)
	{
		//send the metadata
		worker.sock.Send((const char*)&task, sizeof(ConvolveTask<neuronWorker_t>));
		//serialize the images
		for (std::vector<ion::Matrix<neuronWorker_t>>::iterator input_it = task.input->begin(); input_it != task.input->end(); ++input_it)
		{
			input_it->SendToSocket(worker.sock);
		}
		//serialize the kernels
		for (std::vector<ion::Matrix<neuronWorker_t>>::iterator kernel_it = task.kernel->begin(); kernel_it != task.input->end(); ++kernel_it)
		{
			kernel_it->SendToSocket(worker.sock);
		}
		//serialize the results
		for (std::vector<std::vector<ion::Matrix<neuronWorker_t>>>::iterator result_outer_it = task.result->begin(); result_outer_it != task.result->end(); ++result_outer_it)
		{
			for (std::vector<ion::Matrix<neuronWorker_t>>::iterator result_inner_it = result_outer_it->begin(); result_inner_it != result_outer_it->end(); ++result_inner_it)
			{
				result_inner_it->SendToSocketNoData(worker.sock);
			}
		}
		worker.current_task = task;
		return true;
	}
	template <class T>
	void ConvolutionTasker(void* usrdata)
	{
		ion::Error result;
		ConvolveWorkerAnnouncement_t announcement;
		std::vector<ConvolveWorker_t> workers;
		std::vector<ConvolveWorker_t> free_workers;
		std::vector<ConvolveWorker_t> busy_workers;
		ConvolveTask<T> task;
		ConvolutionTaskerData<T>* task_data = (ConvolutionTaskerData<T>*)usrdata;
		ion::UdpSocket announcement_socket;
		announcement_socket.Create();
		announcement_socket.Bind(ion::IpPort(task_data->announcement_port,ion::IpPort::Order::HOST));
		while (true)
		{
			//check for worker announcements
			IpAddress worker_ip;
			result = ion::Error::Get(ion::Error::SUCCESS);
			while (result.success())
			{
				result = announcement_socket.Recv((char*)&announcement, sizeof(ConvolveWorkerAnnouncement_t), 1, &worker_ip, NULL);
				if (result.success())
				{
					//a worker has joined, add them to the pool
					ConvolveWorker_t worker;
					worker.addr = worker_ip;
					worker.work_recv_port = ion::IpPort(announcement.recv_port, ion::IpPort::Order::HOST);
					if (INVALID_CONVOLVE_WORKER != WorkerInPool(workers, worker))
					{
						InitWorker(worker);
						InsertWorker(workers, worker);
						InsertWorker(free_workers, worker);
						LOGINFO("Added convolve worker at address %s port %d to pool", worker.addr.as_string(), worker.work_recv_port.AsHostOrder());
					}
				}
			}
			if (free_workers.size() == 0)
			{
				//cannot do work with no workers
				LOGWARN("There are no available worker threads, convolution cannot proceed");
				ion::ThreadSleep(1000);
				continue;
			}
			//wait for a job
			result = task_data->task_queue.Pop(1, &task);
			if (result.success())
			{
				//a task was provided
				//dispatch the job to an available worker
				ConvolveWorker_t worker = free_workers.back();
				free_workers.pop_back();
				//send the task to this worker
				SendWorkerTask(worker, task);
				busy_workers.push_back(worker);
			}
			//check for replies
			for (std::vector<ConvolveWorker_t>::iterator worker_it = busy_workers.begin(); worker_it != busy_workers.end(); ++worker_it)
			{
				uint64_t magic;
				size_t bytes_read;
				result = worker_it->sock.RecvTimeout((char*)&magic, sizeof(uint64_t), 0, NULL, NULL, &bytes_read);
				if (result.success())
				{
					//we got a reply from this worker
					if (magic == CONVOLVE_TASK_REPLY_MAGIC)
					{
						//this is a valid response, receive all the results
						for (uint32_t result_image_index = 0; result_image_index < worker_it->current_task.num_inputs && result.success(); ++result_image_index)
						{
							for (uint32_t result_kernel_index = 0; result_kernel_index < worker_it->current_task.num_kernels && result.success(); ++result_kernel_index)
							{
								bool recv_result = worker_it->current_task.result->at(result_image_index)[result_kernel_index].RecvFromSocket(worker_it->sock);
								if (!recv_result)
								{
									result = ion::Error::Get(ion::Error::SOCKET);
									//failed to receive response. Put this job back in the queue
									task_data->task_queue.Push(worker_it->current_task);
									LOGWARN("Failed to receive job result, this task is being pushed back into the queue");
								}
							}
						}
					} else
					{
						LOGWARN("Convolve task reply magic failure, expected %llu, received %llu", CONVOLVE_TASK_REPLY_MAGIC, magic);
					}
				} else
				{
					//the worker didn't respond yet
				}

			}
		}
	}
	//template <class T>
	//void InitConvolveThreads(uint32_t num_threads, ConvolutionTaskerData<T>& task_data)
	//{
	//	for (uint32_t thread_index = 0; thread_index < num_threads; ++thread_index)
	//	{
	//		ion::StartThread(ConvolutionTasker<neuronWorker_t>, &task_data);
	//	}
	//}
	ion::Error SendConvolutionWorkerAnnouncement(ion::UdpSocket socket, ion::IpAddress addr, ion::IpPort port, ConvolveWorkerAnnouncement_t announcement, ConvolveWorker_t& worker)
	{

		ion::Error result = socket.SendTo((char*)&announcement, sizeof(ConvolveWorkerAnnouncement_t), addr, port);
		if (!result.success())
		{
			LOGWARN("Failed to send worker announcement");
			return result;
		}
		worker.sock.Create(ion::IpPort(announcement.recv_port,ion::IpPort::Order::HOST), IpAddress((uint32_t)ADDR_ANY));
		result = worker.sock.Listen();
		if (!result.success())
		{
			LOGWARN("Failed to listen worker socket");
			return result;
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	ion::Error GetConvolutionTask(ConvolveWorker_t& worker, ConvolveTask<neuronWorker_t>& task)
	{
		size_t bytes_read;
		ion::Error result = worker.sock.RecvTimeout((char*)&task, sizeof(ConvolveTask<neuronWorker_t>), 1000, NULL, NULL, &bytes_read);
		if (result == ion::Error::Get(ion::Error::TIMEOUT))
		{
			LOGWARN("Timeout occurred receiving task, going to reconnect");
			return result;
		} else if (!result.success())
		{
			LOGWARN("Failed to receive task");
			return result;
		}
		//Construct inputs and outputs
		task.input = new std::vector<ion::Matrix<neuronWorker_t>>;
		task.kernel = new std::vector<ion::Matrix<neuronWorker_t>>;
		task.result = new std::vector<std::vector<ion::Matrix<neuronWorker_t>>>;

		for (uint32_t input_index = 0; input_index < task.num_inputs; ++input_index)
		{
			task.input->push_back(ion::Matrix<neuronWorker_t>(worker.sock, true));
		}
		for (uint32_t kernel_index = 0; kernel_index < task.num_kernels; ++kernel_index)
		{
			task.kernel->push_back(ion::Matrix<neuronWorker_t>(worker.sock, true));
		}
		for (uint32_t input_index = 0; input_index < task.num_inputs; ++input_index)
		{
			task.result->push_back(std::vector<ion::Matrix<neuronWorker_t>>());
			for (uint32_t kernel_index = 0; kernel_index < task.num_kernels; ++kernel_index)
			{
				task.result->back().push_back(ion::Matrix<neuronWorker_t>(worker.sock, false));
			}
		}
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	ion::Error SendConvolutionTaskResult(ConvolveWorker_t& worker)
	{
		for (uint32_t input_index = 0; input_index < worker.current_task.num_inputs; ++input_index)
		{
			for (uint32_t kernel_index = 0; kernel_index < worker.current_task.num_kernels; ++kernel_index)
			{
				worker.current_task.result->at(input_index)[kernel_index].SendToSocket(worker.sock);
			}
		}
		//destroy the allocated matrices
		delete worker.current_task.input;
		delete worker.current_task.kernel;
		delete worker.current_task.result;
		return ion::Error::Get(ion::Error::SUCCESS);
	}
	template <class T>
	void InitConvolutionTasker(uint32_t num_threads, ConvolutionTaskerData<T>& task_data)
	{
		ion::StartThread(ConvolutionTasker<neuronWorker_t>, &task_data);
		LOGINFO("Not yet starting %u threads", num_threads);
	}

	template <class T>
	void PushConvolutionTask(ConvolutionTaskerData<T>& tasker, ConvolveTask<T>& task)
	{

	}

	template <class T>
	void JoinConvolveTasks(ConvolutionTaskerData<T>& tasker)
	{

	}


} //namespace ion
