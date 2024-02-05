// Some easy types
#ifndef PIX_TYPES_H
#define PIX_TYPES_H 1

#ifndef NULL
#ifdef __cplusplus
#define NULL 0 // NULL 4 C++
#else
#define NULL ((void *)0)
#endif
#endif

#if !defined(false)
#define false 0 // Defines False.
#endif
#if !defined(true)
#define true !false // Anything that isn't false, is true
#endif

#if defined _WIN32
typedef unsigned int uint;
#endif
#ifndef PIX_TYPES
#define PIX_TYPES 1

// I need a smarter check then this.
/* #ifndef _LIBCPP_STDINT_H */
/* typedef signed char i8; */
/* typedef unsigned char u8; */
/* typedef signed short i16; */
/* typedef unsigned short u16; */
/* typedef signed int i32; */
/* typedef unsigned int u32; */
/**/
/* #else */
// Use STDINT definitions for portability
#include <stdint.h>
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

// I also want access to size_t, ptrdiff_t and ssize_t
#include <stddef.h>

/* #endif */

typedef float f32;
typedef double f64;
typedef u8 b8;
typedef u16 b16;
typedef i32 b32;

typedef char *string;

// Define struct for String?
// typedef struct __string {
//  char *str;
//  i64 len;
// } String; // ???

#endif

#endif
