#ifndef RVL_SDK_WPAD_INTERNAL_LINT_H
#define RVL_SDK_WPAD_INTERNAL_LINT_H

/* no public header */

/*******************************************************************************
 * macros
 */

#define LINTNextElement(a, b)	((&((a)[b]))[1])

/*******************************************************************************
 * types
 */

#ifdef __cplusplus
	extern "C" {
#endif

typedef unsigned long ULONG;

/*******************************************************************************
 * functions
 */

int LINTCmp(ULONG *lhs, ULONG const *rhs);
void LINTLshift(ULONG *dst, ULONG *src, ULONG shift);
int LINTMsb(ULONG const *data);
void LINTSub(ULONG *dst, ULONG *lhs, ULONG const *rhs);
void LINTMul(ULONG *dst, ULONG *lhs, ULONG const *rhs);

#ifdef __cplusplus
	}
#endif

#endif // RVL_SDK_WPAD_INTERNAL_LINT_H
