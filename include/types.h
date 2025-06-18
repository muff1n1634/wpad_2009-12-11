#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

// NULL

#undef NULL
#define NULL			0

// Fixed-width types

typedef uint8_t			u8;
typedef uint16_t		u16;
typedef uint32_t		u32;
typedef uint64_t		u64;

typedef  int8_t			s8;
typedef  int16_t		s16;
typedef  int32_t		s32;
typedef  int64_t		s64;

// Floating-point types

typedef float			f32;
typedef double			f64;

// Booleans

typedef int				BOOL;
#define TRUE			1
#define FALSE			0

// Byte types

typedef unsigned long int	byte4_t;
typedef unsigned short int	byte2_t;
typedef unsigned char		byte1_t;

typedef byte1_t				byte_t;

// Other types

typedef unsigned long int	register_t;

#endif // TYPES_H
