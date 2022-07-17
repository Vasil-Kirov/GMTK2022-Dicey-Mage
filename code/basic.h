/* date = July 15th 2022 10:45 am */

#ifndef BASIC_H
#define BASIC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#if !defined(true)
#define true 1
#endif

#if !defined(false)
#define false 0
#endif

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define FOR_EACH(arr) for(int i = 0; i < ARR_SIZE(arr); ++i)

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t  b32;
typedef float    f32;
typedef double   f64;

typedef union {
	u32 c;
	struct {
		u8 a;
		u8 b;
		u8 g;
		u8 r;
	};
} Color;

#define _KB(b) (b << 10)
#define _MB(b) (b << 20)
#define _GB(b) (b << 30)

#define KB(b) _KB((i64)b)
#define MB(b) _MB((i64)b)
#define GB(b) _GB((i64)b)


#endif //BASIC_H
