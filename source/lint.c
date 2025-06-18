#define const
#include "lint.h"

/*******************************************************************************
 * headers
 */

#include <limits.h>
#include <string.h>

#include <macros.h> // ATTR_UNUSED

/*******************************************************************************
 * macros
 */

#define ULONG_BIT				(sizeof(ULONG) * CHAR_BIT)
#define ULLONG_BIT				(sizeof(ULLONG) * CHAR_BIT)

#define ULLONG_ULONG_BIT_DIFF	(ULLONG_BIT - ULONG_BIT)

#define ULONG_MSB				((1ul << (ULONG_BIT - 1)))
#define ULLONG_FIRST			((ULLONG)ULONG_MAX + 1)

/*******************************************************************************
 * types
 */

typedef unsigned long long ULLONG;

/*******************************************************************************
 * explanation
 *
 * So basically these LINT* functions operate on these buffers where
 *
 *	struct LINTBuf
 *	{
 *		ULONG	size;
 *		ULONG	data[size];
 *		ULONG	terminator = 0;
 *	};
 *
 * and the data goes backwards (data[0] has the lowest order word, data[size]
 * has the highest).
 *
 * i also tried to make the functions here as type-generic as possible, but some
 * assumptions within the functions may inherently remain that there is a 32-bit
 * wide type and a 64-bit wide type on the machine
 *
 * LINT is likely short for Large INTeger, like bignums.
 */

/*******************************************************************************
 * functions
 */

int LINTCmp(ULONG *lhs, ULONG *rhs)
{
	int i;

	ULONG lhsSize = LINTGetSize(lhs);
	ULONG rhsSize = LINTGetSize(rhs);

	ULONG *lhsData = LINTGetData(lhs);
	ULONG *rhsData = LINTGetData(rhs);

	if (lhsSize > rhsSize)
		return 1;

	if (lhsSize < rhsSize)
		return -1;

	for (i = lhsSize - 1; i >= 0; --i)
	{
		if (lhsData[i] > rhsData[i])
			return 1;

		if (lhsData[i] < rhsData[i])
			return -1;
	}

	return 0;
}

// Good day to you, Xenia
void LINTLshift(ULONG *dst, ULONG *src, ULONG shift)
{
	ULONG i;

	ULONG size = LINTGetSize(src);

	ULONG *srcData = LINTGetData(src);
	ULONG *dstData = LINTGetData(dst);

	ULONG bigShift = shift / ULONG_BIT;
	ULONG smallShift = shift % ULONG_BIT;

	for (i = 0; i < bigShift; ++i)
		dstData[i] = 0;

	ULLONG tmp = 0;

	for (i = 0; i < size; ++i)
	{
		tmp += (ULLONG)srcData[i] << smallShift;

		dstData[i + bigShift] = tmp & ULONG_MAX;
		tmp = (tmp >> ULLONG_ULONG_BIT_DIFF) & ULONG_MAX;
	}

	dstData[i + bigShift] = tmp;

	// recalculating size
	LINTSetSize(dst, size + bigShift);

	if (tmp)
		++(*dst); // would be LINTSetSize(dst, LINTGetSize(dst) + 1);
}

int LINTMsb(ULONG *num)
{
	ULONG i;

	ULONG size = LINTGetSize(num);

	ULONG last = *LINTNextElement(num, size - 1);
	int msbPos = ULONG_BIT;

	// NOTE: msbPos-- is explicitly post-decrement
	for (i = 0; i < ULONG_BIT; ++i, msbPos--)
	{
		if ((ULONG_MSB >> i) & last)
			break;
	}

	return msbPos + (size - 1) * ULONG_BIT;
}

void LINTAdd(ULONG *dst, ULONG *lhs, ULONG *rhs)
{
	ULONG i;

	ULONG lhsSize = LINTGetSize(lhs);
	ULONG rhsSize = LINTGetSize(rhs);

	ULONG *lhsData = LINTGetData(lhs);
	ULONG *rhsData = LINTGetData(rhs);
	ULONG *dstData = LINTGetData(dst);

	ULLONG tmp = 0;

	for (i = 0; i < lhsSize || i < rhsSize; ++i)
	{
		if (i < lhsSize)
			tmp += lhsData[i];

		if (i < rhsSize)
			tmp += rhsData[i];

		dstData[i] = tmp & ULONG_MAX;

		tmp = tmp / ULLONG_FIRST & ULONG_MAX;
	}

	dstData[i] = (ULONG)tmp;

	// recalculating size
	dstData[i] == 0 ? LINTSetSize(dst, i) : LINTSetSize(dst, i + 1);
}

void LINTSub(ULONG *dst, ULONG *lhs, ULONG *rhs)
{
	ULONG i;

	ULONG lhsSize = LINTGetSize(lhs);
	ULONG rhsSize = LINTGetSize(rhs);

	ULONG *lhsData = LINTGetData(lhs);
	ULONG *rhsData = LINTGetData(rhs);
	ULONG *dstData = LINTGetData(dst);

	ULLONG tmp = 0;

	for (i = 0; i < lhsSize; ++i)
	{
		dstData[i] = lhsData[i] - (ULONG)tmp;

		if (dstData[i] > ULONG_MAX - tmp)
			tmp = 1;
		else
			tmp = 0;

		if (i < rhsSize)
			dstData[i] -= rhsData[i];

		if (dstData[i] > ULONG_MAX - rhsData[i])
			++tmp;
	}

	// recalculating size
	i = lhsSize;
	while (i && !dstData[i])
		--i;

	LINTSetSize(dst, i + 1);
}

// TODO on release
#pragma push

void LINTMul(ULONG *dst, ULONG *lhs, ULONG *rhs)
{
	ULONG i;
	ULONG j;

	ULONG lhsSize = LINTGetSize(lhs);
	ULONG rhsSize = LINTGetSize(rhs);

	ULONG *lhsData = LINTGetData(lhs);
	ULONG *rhsData = LINTGetData(rhs);
	ULONG *dstData = LINTGetData(dst);

	ULLONG tmp = 0;

	LINTSetSize(dst, 1);

	// TODO: where is 32 from? (30 + 2?)
	memset(LINTGetData(dst), 0, sizeof(ULONG) * 32);

	// either number is 0
	if ((LINTGetSize(lhs) == 1 && LINTGetData(lhs)[0] == 0)
	    || (LINTGetSize(rhs) == 1 && LINTGetData(rhs)[0] == 0))
	{
		return;
	}

	for (i = 0; i < rhsSize; ++i)
	{
		tmp = 0;

		for (j = 0; j < lhsSize; ++j)
		{
			tmp =
				(ULLONG)tmp + (ULLONG)lhsData[j] * rhsData[i] + dstData[i + j];

			dstData[i + j] = tmp & ULONG_MAX;

#if !defined(NDEBUG)
			tmp = (tmp >> ULLONG_ULONG_BIT_DIFF & ULONG_MAX) % ULLONG_FIRST;
#else // TODO
			tmp >>= ULLONG_ULONG_BIT_DIFF;
			tmp &= ULONG_MAX;
			tmp %= ULLONG_FIRST;
#endif
		}

		dstData[i + j] = tmp;
	}

	// recalculating size
	tmp == 0 ? LINTSetSize(dst, lhsSize + rhsSize - 1)
			 : LINTSetSize(dst, lhsSize + rhsSize);
}

#pragma pop

void LINTMod(ULONG *dst, ULONG *lhs, ULONG *rhs)
{
	int i;

	// force of habit, i'm sure
	ULONG lhsSize ATTR_UNUSED = LINTGetSize(lhs);
	ULONG rhsSize ATTR_UNUSED = LINTGetSize(rhs);

	ULONG tmpLint[LINT_NUM_MAX_BUFSIZ];

	int msbDiff = LINTMsb(lhs) - LINTMsb(rhs);

	memcpy(dst, lhs, sizeof(ULONG) * LINT_NUM_MAX_BUFSIZ);

	for (i = msbDiff; i >= 0; --i)
	{
		LINTLshift(tmpLint, rhs, i);

		if (LINTCmp(dst, tmpLint) >= 0)
			LINTSub(dst, dst, tmpLint);
	}

	if (LINTCmp(dst, tmpLint) >= 0)
		LINTSub(dst, dst, rhs);
}

void LINTAddMod(ULONG *dst, ULONG *add1, ULONG *add2, ULONG *mod)
{
	LINTAdd(dst, add1, add2);

	// Hello. Have you heard of the >= operator. I think your neighbor has
	if (LINTCmp(dst, mod) > 0 || LINTCmp(dst, mod) == 0)
		LINTSub(dst, dst, mod);
}
