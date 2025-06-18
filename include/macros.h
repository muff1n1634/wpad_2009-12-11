#ifndef MACROS_H
#define MACROS_H

#define STR_(x)					#x
#define STR(x)					STR_(x)

#define AT_ADDRESS(x)			: (x)

#define MIN(x, y)				((x) < (y) ? (x) : (y))

#define ROUND_UP(x, align)		(((x) + ((align) - 1)) & -(align))

#define IS_ALIGNED(x, align)	(((u32)(x) & ((align) - 1)) == 0)

#define ARRAY_LENGTH(x)			(sizeof (x) / sizeof ((x)[0]))

#define BOOLIFY_TERNARY(expr_)			((expr_) ? 1 : 0)
#define BOOLIFY_TERNARY_FALSE(expr_)	((expr_) ? 0 : 1)

#endif // MACROS_H
