#include "ionlib/matrix_arithmetic.h"
#include "ionlib/matrix_basic.h"
#include "ionlib/matrix_filtering.h"
#include "ionlib/matrix_serialization.h"
//explicitly instantiate this class for type double. This is what lets us put the implementations into a CPP file instead of a header file
template class ion::Matrix<double>;
template class ion::Matrix<uint32_t>;
template class ion::Matrix<uint8_t>;
template struct ion::ConvolveTask<double>;
template struct ion::ConvolveTask<uint32_t>;
template struct ion::ConvolveTask<uint8_t>;
template struct ion::ConvolveTaskData<double>;
template struct ion::ConvolveTaskData<uint32_t>;
template struct ion::ConvolveTaskData<uint8_t>;
