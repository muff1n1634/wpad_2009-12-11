#include <revolution/WPAD/WPADClamp.h>
// no internal header

/*******************************************************************************
 * headers
 */

#include <math.h>

#include <types.h>

#include <revolution/WPAD/WPAD.h>

#include "context_rvl.h"

/*******************************************************************************
 * macros
 */

#undef NULL
#define NULL	((void *)(0))

/*******************************************************************************
 * types
 */

typedef u8 ClampStickUnit;
enum ClampStickUnit_et
{
	STICK_UNIT_S8,
	STICK_UNIT_S16,
};

/*******************************************************************************
 * local function declarations
 */

static void __ClampStickOctagon(void *x, void *y, s16, s16, s16,
                                ClampStickUnit type);
static void __ClampStickCircle(void *x, void *y, s16, s16, ClampStickUnit type);
static void __ClampTrigger(u8 *t, u8, u8);
static void __ClampCube(s16 *x, s16 *y, s16 *z, WPADAccGravityUnit acc, f32);
static void __ClampSphere(s16 *x, s16 *y, s16 *z, WPADAccGravityUnit acc, f32);

/*******************************************************************************
 * variables
 */

// .data
static s16 const FSRegion[4] = {15, 67, 43, 56};
static s16 const FSRegion2[4] = {0, 82, 58, 75};
static s16 const CLRegion[4] = {60, 288, 188, 248};
static s16 const CLRegion2[4] = {0, 348, 248, 322};

/*******************************************************************************
 * functions
 */

void WPADClampStick(WPADChannel chan, WPADStatus *status, WPADClampType type)
{
	WPADFSStatus *fs;
	WPADCLStatus *cl;

	s16 const *region;

	OSAssert_Line(113, status != NULL);

	if (status->err != WPAD_ESUCCESS)
		return;

	WPADDataFormat fmt = WPADGetDataFormat(chan);
	switch (fmt)
	{
	case WPAD_FMT_FS_BTN:
	case WPAD_FMT_FS_BTN_ACC:
	case WPAD_FMT_FS_BTN_ACC_DPD:
		fs = (WPADFSStatus *)status;

		if (type == WPAD_CLAMP_STICK_OCTAGON_1
		    || type == WPAD_CLAMP_STICK_CIRCLE)
		{
			region = FSRegion;
		}
		else // if (type == WPAD_CLAMP_STICK_OCTAGON_2)
		{
			region = FSRegion2;
		}

		if (type == WPAD_CLAMP_STICK_OCTAGON_1
		    || type == WPAD_CLAMP_STICK_OCTAGON_2)
		{
			__ClampStickOctagon(&fs->fsStickX, &fs->fsStickY, region[1],
			                    region[2], region[0], STICK_UNIT_S8);
		}
		else // if (type == WPAD_CLAMP_STICK_CIRCLE)
		{
			__ClampStickCircle(&fs->fsStickX, &fs->fsStickY, region[3],
			                   region[0], STICK_UNIT_S8);
		}

		break;

	case WPAD_FMT_CLASSIC_BTN:
	case WPAD_FMT_CLASSIC_BTN_ACC:
	case WPAD_FMT_CLASSIC_BTN_ACC_DPD:
		cl = (WPADCLStatus *)status;

		if (type == WPAD_CLAMP_STICK_OCTAGON_1
		    || type == WPAD_CLAMP_STICK_CIRCLE)
		{
			region = CLRegion;
		}
		else // if (type == WPAD_CLAMP_STICK_OCTAGON_2)
		{
			region = CLRegion2;
		}

		if (type == WPAD_CLAMP_STICK_OCTAGON_1
		    || type == WPAD_CLAMP_STICK_OCTAGON_2)
		{
			__ClampStickOctagon(&cl->clLStickX, &cl->clLStickY, region[1],
			                    region[2], region[0], STICK_UNIT_S16);
			__ClampStickOctagon(&cl->clRStickX, &cl->clRStickY, region[1],
			                    region[2], region[0], STICK_UNIT_S16);
		}
		else // if (type == WPAD_CLAMP_STICK_CIRCLE)
		{
			__ClampStickCircle(&cl->clLStickX, &cl->clLStickY, region[3],
			                   region[0], STICK_UNIT_S16);
			__ClampStickCircle(&cl->clRStickX, &cl->clRStickY, region[3],
			                   region[0], STICK_UNIT_S16);
		}

		break;
	}
}

void WPADClampTrigger(WPADChannel chan, WPADStatus *status, WPADClampType type)
{
	WPADCLStatus *cl;

	u8 a;
	u8 b;
	u8 c;
	u8 d;

	OSAssert_Line(227, status != NULL);

	// status needs another use so I just put it here
	if (((WPADStatus *)status)->err != WPAD_ESUCCESS)
		return;

	WPADDataFormat fmt = WPADGetDataFormat(chan);
	switch (fmt)
	{
	case WPAD_FMT_CLASSIC_BTN:
	case WPAD_FMT_CLASSIC_BTN_ACC:
	case WPAD_FMT_CLASSIC_BTN_ACC_DPD:
		cl = (WPADCLStatus *)status;

		if (type == WPAD_CLAMP_TRIGGER_TYPE_1)
		{
			a = b = 30;
			c = d = 180;
		}
		else // if (type == WPAD_CLAMP_TRIGGER_TYPE_2)
		{
			WPADGetCLTriggerThreshold(chan, &a, &b);

			c = a < 76 ? a + 180 : 255;
			d = b < 76 ? b + 180 : 255;
		}

		__ClampTrigger(&cl->clTriggerL, a, c);
		__ClampTrigger(&cl->clTriggerR, b, d);

		break;
	}
}

void WPADClampAcc(WPADChannel chan, WPADStatus *status, WPADClampType type)
{
	WPADStatus *cr; // core
	WPADFSStatus *fs;

	WPADAccGravityUnit coreAcc;
	WPADAccGravityUnit fsAcc;

	OSAssert_Line(282, status != NULL);

	if (status->err != WPAD_ESUCCESS)
		return;

	WPADDataFormat fmt = WPADGetDataFormat(chan);
	switch (fmt)
	{
	case WPAD_FMT_CORE_BTN_ACC:
	case WPAD_FMT_CORE_BTN_ACC_DPD:
		cr = (WPADStatus *)status;

		WPADGetAccGravityUnit(chan, WPAD_ACC_GRAVITY_UNIT_CORE, &coreAcc);

		if (type == WPAD_CLAMP_ACC_CUBE)
			__ClampCube(&cr->accX, &cr->accY, &cr->accZ, coreAcc, 3.4f);
		else // if (type == WPAD_CLAMP_ACC_SPHERE)
			__ClampSphere(&cr->accX, &cr->accY, &cr->accZ, coreAcc, 3.4f);

		break;

	case WPAD_FMT_FS_BTN_ACC:
	case WPAD_FMT_FS_BTN_ACC_DPD:
		fs = (WPADFSStatus *)status;

		WPADGetAccGravityUnit(chan, WPAD_ACC_GRAVITY_UNIT_CORE, &coreAcc);
		WPADGetAccGravityUnit(chan, WPAD_ACC_GRAVITY_UNIT_FS, &fsAcc);

		if (type == WPAD_CLAMP_ACC_CUBE)
		{
			__ClampCube(&fs->accX, &fs->accY, &fs->accZ, coreAcc, 3.4f);
			__ClampCube(&fs->fsAccX, &fs->fsAccY, &fs->fsAccZ, fsAcc, 2.1f);
		}
		else // if (type == WPAD_CLAMP_ACC_SPHERE)
		{
			__ClampSphere(&fs->accX, &fs->accY, &fs->accZ, coreAcc, 3.4f);
			__ClampSphere(&fs->fsAccX, &fs->fsAccY, &fs->fsAccZ, fsAcc, 2.1f);
		}

		break;
	}
}

static void __ClampStickOctagon(void *x, void *y, s16 param_3, s16 param_4,
                                s16 param_5, ClampStickUnit unit)
{
	int myX, myY;

	int a;
	int b;
	int c;

	if (unit == STICK_UNIT_S8)
	{
		myX = *(s8 *)x;
		myY = *(s8 *)y;
	}
	else // if (unit == STICK_UNIT_S16)
	{
		myX = *(s16 *)x;
		myY = *(s16 *)y;
	}

	if (myX >= 0)
	{
		a = 1;
	}
	else
	{
		a = -1;
		myX = -myX;
	}

	if (myY >= 0)
	{
		b = 1;
	}
	else
	{
		b = -1;
		myY = -myY;
	}

	if (myX <= param_5)
		myX = 0;
	else
		myX -= param_5;

	if (myY <= param_5)
		myY = 0;
	else
		myY -= param_5;

	if (myX == 0 && myY == 0)
	{
		if (unit == STICK_UNIT_S8)
		{
			*(s8 *)x = 0;
			*(s8 *)y = 0;
		}
		else // if (unit == STICK_UNIT_S16)
		{
			*(s16 *)x = 0;
			*(s16 *)y = 0;
		}

		return;
	}

	if (param_4 * myY <= param_4 * myX)
	{
		c = param_4 * myX + (param_3 - param_4) * myY;

		if (param_4 * param_3 < c)
		{
			if (unit == STICK_UNIT_S8)
			{
				myX = (s8)(param_4 * param_3 * myX / c);
				myY = (s8)(param_4 * param_3 * myY / c);
			}
			else // if (unit == STICK_UNIT_S16)
			{
				myX = (s16)(param_4 * param_3 * myX / c);
				myY = (s16)(param_4 * param_3 * myY / c);
			}
		}
	}
	else
	{
		c = param_4 * myY + (param_3 - param_4) * myX;

		if (param_4 * param_3 < c)
		{
			if (unit == STICK_UNIT_S8)
			{
				myX = (s8)(param_4 * param_3 * myX / c);
				myY = (s8)(param_4 * param_3 * myY / c);
			}
			else // if (unit == STICK_UNIT_S16)
			{
				myX = (s16)(param_4 * param_3 * myX / c);
				myY = (s16)(param_4 * param_3 * myY / c);
			}
		}
	}

	if (unit == STICK_UNIT_S8)
	{
		*(s8 *)x = a * myX;
		*(s8 *)y = b * myY;
	}
	else // if (unit == STICK_UNIT_S16)
	{
		*(s16 *)x = a * myX;
		*(s16 *)y = b * myY;
	}
}

static void __ClampStickCircle(void *x, void *y, s16 param_3, s16 param_4,
                               ClampStickUnit unit)
{
	int myX, myY;

	int sqrd, sqrt;

	if (unit == STICK_UNIT_S8)
	{
		myX = *(s8 *)x;
		myY = *(s8 *)y;
	}
	else // if (unit == STICK_UNIT_S16)
	{
		myX = *(s16 *)x;
		myY = *(s16 *)y;
	}

	if (-param_4 < myX && myX < param_4)
		myX = 0;
	else if (myX > 0)
		myX -= param_4;
	else
		myX += param_4;

	if (-param_4 < myY && myY < param_4)
		myY = 0;
	else if (myY > 0)
		myY -= param_4;
	else
		myY += param_4;

	sqrd = myX * myX + myY * myY;

	if (param_3 * param_3 < sqrd)
	{
		sqrt = sqrtf(sqrd);

		myX = myX * param_3 / sqrt;
		myY = myY * param_3 / sqrt;
	}

	if (unit == STICK_UNIT_S8)
	{
		*(s8 *)x = myX;
		*(s8 *)y = myY;
	}
	else // if (unit == STICK_UNIT_S16)
	{
		*(s16 *)x = myX;
		*(s16 *)y = myY;
	}
}

static void __ClampTrigger(u8 *t, u8 param_2, u8 param_3)
{
	if (*t <= param_2)
	{
		*t = 0;

		return;
	}

	if (param_3 < *t)
		*t = param_3;

	*t -= param_2;
}

static void __ClampCube(s16 *x, s16 *y, s16 *z,
                        WPADAccGravityUnit acc, f32 param_5)
{
	f32 a;
	f32 b;
	f32 c;
	f32 d;
	f32 e;
	f32 f;

	a = (f32)*x / (f32)acc.x;
	b = (f32)*y / (f32)acc.y;
	c = (f32)*z / (f32)acc.z;

	d = 1.0f;
	e = 1.0f;
	f = 1.0f;

	if (a < 0.0f)
	{
		d = -1.0f;
		a = -a;
	}

	if (b < 0.0f)
	{
		e = -1.0f;
		b = -b;
	}

	if (c < 0.0f)
	{
		f = -1.0f;
		c = -c;
	}

	if (a > param_5)
		a = param_5;

	if (b > param_5)
		b = param_5;

	if (c > param_5)
		c = param_5;

	a *= d;
	b *= e;
	c *= f;

	*x = a * acc.x;
	*y = b * acc.y;
	*z = c * acc.z;
}

static void __ClampSphere(s16 *x, s16 *y, s16 *z, WPADAccGravityUnit acc,
                          f32 param_5)
{
	f32 a;
	f32 b;
	f32 c;
	f32 d;
	f32 e;

	c = (f32)*x / (f32)acc.x;
	d = (f32)*y / (f32)acc.y;
	e = (f32)*z / (f32)acc.z;
	a = c * c + d * d + e * e;

	if (param_5 * param_5 < a)
	{
		b = sqrtf(a);

		c = c * param_5 / b;
		d = d * param_5 / b;
		e = e * param_5 / b;
	}

	*x = c * acc.x;
	*y = d * acc.y;
	*z = e * acc.z;
}
