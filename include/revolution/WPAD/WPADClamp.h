#ifndef RVL_SDK_WPAD_CLAMP_H
#define RVL_SDK_WPAD_CLAMP_H

/*******************************************************************************
 * headers
 */

#include <types.h> // u32

#include "WPAD.h"

/*******************************************************************************
 * types
 */

#ifdef __cplusplus
	extern "C" {
#endif

typedef u32 WPADClampType;
enum WPADClampType_et
{
	WPAD_CLAMP_STICK_OCTAGON_1 = 0,
	WPAD_CLAMP_STICK_OCTAGON_2,
	WPAD_CLAMP_STICK_CIRCLE,

	WPAD_CLAMP_TRIGGER_TYPE_1 = 0,
	WPAD_CLAMP_TRIGGER_TYPE_2,

	WPAD_CLAMP_ACC_CUBE = 0,
	WPAD_CLAMP_ACC_SPHERE,
};

/*******************************************************************************
 * functions
 */

void WPADClampStick(WPADChannel chan, WPADStatus *status, WPADClampType type);
void WPADClampTrigger(WPADChannel chan, WPADStatus *status, WPADClampType type);
void WPADClampAcc(WPADChannel chan, WPADStatus *status, WPADClampType type);

#ifdef __cplusplus
	}
#endif

#endif // RVL_SDK_WPAD_CLAMP_H
