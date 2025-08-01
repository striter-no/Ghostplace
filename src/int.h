#pragma once
#include <stdint.h>
#include <float.h>

#define ubyte uint8_t
#define byte  int8_t

#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define f32 float
#define f64 double

/* Max values 
x - maX
m - Min
*/

#define x_ubyte UINT8_MAX

#define m_byte  INT8_MIN
#define m_i16   INT16_MIN
#define m_i32   INT32_MIN
#define m_i64   INT64_MIN

#define x_byte  INT8_MAX
#define x_i16   INT16_MAX
#define x_i32   INT32_MAX
#define x_i64   INT64_MAX

#define x_u16   UINT16_MAX
#define x_u32   UINT32_MAX
#define x_u64   UINT64_MAX

#define m_f32   FLT_MIN
#define m_f64   DBL_MIN

#define x_f32   FLT_MAX
#define x_f64   DBL_MAX

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a))
#endif