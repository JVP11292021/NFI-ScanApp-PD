/** @file imopv_inst.c
 ** @brief Instantiate imopv.c for multiple types
 **/

#define VL_TYPE_FLOAT  float
#define FLT VL_TYPE_FLOAT
#define VL_IMOPV_INSTANTIATING
#include "imopv.c"

#define VL_TYPE_DOUBLE double
#define FLT VL_TYPE_DOUBLE
#define VL_IMOPV_INSTANTIATING
#include "imopv.c"

#define VL_TYPE_UINT32 uint32_t
#define FLT VL_TYPE_UINT32
#define VL_IMOPV_INSTANTIATING
#include "imopv.c"

#define VL_TYPE_INT32  int32_t
#define FLT VL_TYPE_INT32
#define VL_IMOPV_INSTANTIATING
#include "imopv.c"
