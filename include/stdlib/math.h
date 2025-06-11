#ifndef __STDC_MATH_H__
#define __STDC_MATH_H__

extern double atan(double x);

extern double cos(double x);
extern double sin(double x);
extern double tan(double x);

extern double sqrt(double x);

extern float sqrtf(float x);

/* Technically NDEBUG is only supposed to control the definition of the assert()
 * macro in <assert.h>, but it is still a standard macro (and one this
 * repository uses).
 */
#if defined(NDEBUG)
inline float sqrtf(float x) { return (float)(sqrt)(x); }
#endif

#endif // __STDC_MATH_H__
