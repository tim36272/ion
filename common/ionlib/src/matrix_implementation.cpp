#include "ionlib/matrix_arithmetic.h"
#include "ionlib/matrix_basic.h"
#include "ionlib/matrix_filtering.h"
#include "ionlib/matrix_serialization.h"
#include "ionlib/matrix_opencv.h"
//explicitly instantiate this class for type double. This is what lets us put the implementations into a CPP file instead of a header file
template class ion::Matrix<double>;
template class ion::Matrix<float>;
template class ion::Matrix<uint32_t>;
template class ion::Matrix<uint8_t>;
template struct ion::ConvolveTask<double>;
template struct ion::ConvolveTask<float>;
template struct ion::ConvolveTask<uint32_t>;
template struct ion::ConvolveTask<uint8_t>;
template struct ion::ConvolutionTaskerData<double>;
template struct ion::ConvolutionTaskerData<float>;
template struct ion::ConvolutionTaskerData<uint32_t>;
template struct ion::ConvolutionTaskerData<uint8_t>;

//explict instantiations from matrix_arithmetic.h
//double
template ion::Matrix<double> ion::operator+(const ion::Matrix<double>& lhs, const ion::Matrix<double>& rhs);
template ion::Matrix<double> ion::operator+(const ion::Matrix<double>& lhs, double rhs);
template ion::Matrix<double> ion::operator-(const ion::Matrix<double>& lhs, const ion::Matrix<double>& rhs);
template ion::Matrix<double> ion::operator*(const ion::Matrix<double>& lhs, const ion::Matrix<double>& rhs);
template ion::Matrix<double> ion::operator*(const ion::Matrix<double>& lhs, double rhs);
template ion::Matrix<double> ion::operator/(const ion::Matrix<double>& lhs, double rhs);
//float
template ion::Matrix<float> ion::operator+(const ion::Matrix<float>& lhs, const ion::Matrix<float>& rhs);
template ion::Matrix<float> ion::operator+(const ion::Matrix<float>& lhs, float rhs);
template ion::Matrix<float> ion::operator-(const ion::Matrix<float>& lhs, const ion::Matrix<float>& rhs);
template ion::Matrix<float> ion::operator*(const ion::Matrix<float>& lhs, const ion::Matrix<float>& rhs);
template ion::Matrix<float> ion::operator*(const ion::Matrix<float>& lhs, float rhs);
template ion::Matrix<float> ion::operator/(const ion::Matrix<float>& lhs, float rhs);
//uint32_t
template ion::Matrix<uint8_t> ion::operator+(const ion::Matrix<uint8_t>& lhs, const ion::Matrix<uint8_t>& rhs);
template ion::Matrix<uint8_t> ion::operator+(const ion::Matrix<uint8_t>& lhs, uint8_t rhs);
template ion::Matrix<uint8_t> ion::operator-(const ion::Matrix<uint8_t>& lhs, const ion::Matrix<uint8_t>& rhs);
template ion::Matrix<uint8_t> ion::operator*(const ion::Matrix<uint8_t>& lhs, const ion::Matrix<uint8_t>& rhs);
template ion::Matrix<uint8_t> ion::operator*(const ion::Matrix<uint8_t>& lhs, uint8_t rhs);
template ion::Matrix<uint8_t> ion::operator/(const ion::Matrix<uint8_t>& lhs, uint8_t rhs);
//uint8_t
template ion::Matrix<uint32_t> ion::operator+(const ion::Matrix<uint32_t>& lhs, const ion::Matrix<uint32_t>& rhs);
template ion::Matrix<uint32_t> ion::operator+(const ion::Matrix<uint32_t>& lhs, uint32_t rhs);
template ion::Matrix<uint32_t> ion::operator-(const ion::Matrix<uint32_t>& lhs, const ion::Matrix<uint32_t>& rhs);
template ion::Matrix<uint32_t> ion::operator*(const ion::Matrix<uint32_t>& lhs, const ion::Matrix<uint32_t>& rhs);
template ion::Matrix<uint32_t> ion::operator*(const ion::Matrix<uint32_t>& lhs, uint32_t rhs);
template ion::Matrix<uint32_t> ion::operator/(const ion::Matrix<uint32_t>& lhs, uint32_t rhs);

//explicit instantiations from matrix_basic.h
//double
template ion::Matrix<double> ion::Linspace(double min, double max);
//double to uint32_t
template void ion::Matrix<double>::Cast(const ion::Matrix<uint32_t>& rhs);
//double to uint8_t
template void ion::Matrix<double>::Cast(const ion::Matrix<uint8_t>& rhs);
//float
template ion::Matrix<float> ion::Linspace(float min, float max);
//float to uint32_t
template void ion::Matrix<float>::Cast(const ion::Matrix<uint32_t>& rhs);
//float to uint8_t
template void ion::Matrix<float>::Cast(const ion::Matrix<uint8_t>& rhs);
//uint8_t
template ion::Matrix<uint8_t> ion::Linspace(uint8_t min, uint8_t max);
//uint32_t
template ion::Matrix<uint32_t> ion::Linspace(uint32_t min, uint32_t max);

//explicit instantiations from matrix_filtering.h
//double
template ion::Matrix<double> ion::Convolve(const ion::Matrix<double>& mat, const ion::Matrix<double>& kernel, ion::Matrix<double>::ConvFlag flags);
template ion::Matrix<double> ion::ConvolveDryRun(const ion::Matrix<double>& mat, const ion::Matrix<double>& kernel, ion::Matrix<double>::ConvFlag flags);
template ion::Matrix<double> ion::MaxPool(const ion::Matrix<double>& mat, uint32_t pool_size);
template ion::Matrix<double> ion::Softmax(const ion::Matrix<double>& mat);
template void				 ion::InitConvolutionTasker(uint32_t num_threads, ConvolutionTaskerData<double>& task_data);
//float
template ion::Matrix<float> ion::Convolve(const ion::Matrix<float>& mat, const ion::Matrix<float>& kernel, ion::Matrix<float>::ConvFlag flags);
template ion::Matrix<float> ion::ConvolveDryRun(const ion::Matrix<float>& mat, const ion::Matrix<float>& kernel, ion::Matrix<float>::ConvFlag flags);
template ion::Matrix<float> ion::MaxPool(const ion::Matrix<float>& mat, uint32_t pool_size);
template ion::Matrix<float> ion::Softmax(const ion::Matrix<float>& mat);
template void				 ion::InitConvolutionTasker(uint32_t num_threads, ConvolutionTaskerData<float>& task_data);
//uint8_t
template ion::Matrix<uint8_t> ion::Convolve(const ion::Matrix<uint8_t>& mat, const ion::Matrix<uint8_t>& kernel, ion::Matrix<uint8_t>::ConvFlag flags);
template ion::Matrix<uint8_t> ion::ConvolveDryRun(const ion::Matrix<uint8_t>& mat, const ion::Matrix<uint8_t>& kernel, ion::Matrix<uint8_t>::ConvFlag flags);
template ion::Matrix<uint8_t> ion::MaxPool(const ion::Matrix<uint8_t>& mat, uint32_t pool_size);
template ion::Matrix<uint8_t> ion::Softmax(const ion::Matrix<uint8_t>& mat);
template void				  ion::InitConvolutionTasker(uint32_t num_threads, ConvolutionTaskerData<uint8_t>& task_data);
//uint32_t
template ion::Matrix<uint32_t> ion::Convolve(const ion::Matrix<uint32_t>& mat, const ion::Matrix<uint32_t>& kernel, ion::Matrix<uint32_t>::ConvFlag flags);
template ion::Matrix<uint32_t> ion::ConvolveDryRun(const ion::Matrix<uint32_t>& mat, const ion::Matrix<uint32_t>& kernel, ion::Matrix<uint32_t>::ConvFlag flags);
template ion::Matrix<uint32_t> ion::MaxPool(const ion::Matrix<uint32_t>& mat, uint32_t pool_size);
template ion::Matrix<uint32_t> ion::Softmax(const ion::Matrix<uint32_t>& mat);
template void				   ion::InitConvolutionTasker(uint32_t num_threads, ConvolutionTaskerData<uint32_t>& task_data);

//Explict instantiations from matrix_serialization.h
//double
template std::ostream& ion::operator<< (std::ostream& out, const ion::Matrix<double>& mat);
//float
template std::ostream& ion::operator<< (std::ostream& out, const ion::Matrix<float>& mat);
//uint8_t
template std::ostream& ion::operator<< (std::ostream& out, const ion::Matrix<uint8_t>& mat);
//uint32_t
template std::ostream& ion::operator<< (std::ostream& out, const ion::Matrix<uint32_t>& mat);
