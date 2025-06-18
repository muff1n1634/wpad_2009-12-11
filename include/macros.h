#ifndef MACROS_H
#define MACROS_H

// macro helpers

#define STR_(x)							#x
#define STR(x)							STR_(x)

// keywords

#ifndef alignas
# define alignas						ATTR_ALIGN
#endif

// attributes

#define ATTR_ALIGN(x)					__attribute__((aligned(x)))
#define ATTR_FALLTHROUGH				/* no known attribute, but mwcc doesn't seem to care */
#define ATTR_UNUSED						__attribute__((unused))
#define ATTR_WEAK						__attribute__((weak))

#if defined(__MWERKS__)
# define AT_ADDRESS(x)					: x
#else
# define AT_ADDRESS(x)
#endif

// useful stuff

#define MIN(x, y)						((x) < (y) ? (x) : (y))

#define ROUND_UP(x, align)				(((x) + ((align) - 1)) & -(align))

#define IS_ALIGNED(x, align)			(((unsigned long)(x) & ((align) - 1)) == 0)

#define ARRAY_LENGTH(x)					(sizeof (x) / sizeof ((x)[0]))

#define BOOLIFY_TERNARY(expr_)			((expr_) ? 1 : 0)
#define BOOLIFY_TERNARY_FALSE(expr_)	((expr_) ? 0 : 1)

#endif // MACROS_H
