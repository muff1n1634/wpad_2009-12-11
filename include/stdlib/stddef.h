#ifndef __STDC_STDDEF_H__
#define __STDC_STDDEF_H__

#include <__internal/__size_t.h>
#include <__internal/__NULL.h>

#define offsetof(type, member_designator)	((size_t)&(((type *)0)->member_designator))

#endif // __STDC_STDDEF_H__
