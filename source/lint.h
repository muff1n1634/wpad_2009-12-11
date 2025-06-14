#ifndef RVL_SDK_WPAD_INTERNAL_LINT_H
#define RVL_SDK_WPAD_INTERNAL_LINT_H

/* no public header(?) */

/*******************************************************************************
 * macros
 */

// Largest number buffer this library seems to operate on
#define LINT_NUM_MAX_LENGTH		64
#define LINT_NUM_MAX_BUFSIZ		(1 + LINT_NUM_MAX_LENGTH + 1)

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

// returns ULONG
#define LINTGetSize(lint_)				(*(lint_))

// returns ULONG *
#define LINTGetData(lint_)				((lint_) + 1)

// returns ULONG
#define LINTSetSize(lint_, size_)		(*(lint_) = (size_))

// returns ULONG *
#define LINTNextElement(lint_, index_)	((lint_) + (index_) + 1)

int LINTCmp(ULONG *lhs, ULONG const *rhs);
void LINTLshift(ULONG *dst, ULONG const *src, ULONG shift);
int LINTMsb(ULONG const *num);
void LINTAdd(ULONG *dst, ULONG *lhs, ULONG *rhs);
void LINTSub(ULONG *dst, ULONG *lhs, ULONG const *rhs);
void LINTMul(ULONG *dst, ULONG *lhs, ULONG const *rhs);
void LINTMod(ULONG *dst, ULONG *lhs, ULONG *rhs);
void LINTAddMod(ULONG *dst, ULONG *add1, ULONG *add2, ULONG *mod);

#ifdef __cplusplus
	}
#endif

#endif // RVL_SDK_WPAD_INTERNAL_LINT_H
