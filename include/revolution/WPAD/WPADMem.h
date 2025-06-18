#ifndef RVL_SDK_WPAD_MEMORY_H
#define RVL_SDK_WPAD_MEMORY_H

/*******************************************************************************
 * headers
 */

#include <types.h>

#include "WPAD.h"

#if 0
#include <revolution/OS/OSTime.h>
#endif

#include <context_rvl.h>

/*******************************************************************************
 * macros
 */

#define WM_MEM_ADDR(addr_)	((addr_) & 0xffff)
#define WM_EXT_REG_ADDR(type_, addr_)	\
	(((addr_) & 0xffff) | ((WPAD_EXT_REG_ ## type_) << 16) | (1 << 26))

/*******************************************************************************
 * types
 */

#ifdef __cplusplus
	extern "C" {
#endif

typedef u8 WPADExtRegType;
enum WPADExtRegType_et
{
	WPAD_EXT_REG_SPEAKER		= 0xa2,
	WPAD_EXT_REG_EXTENSION		= 0xa4,
	WPAD_EXT_REG_MOTION_PLUS	= 0xa6,
	WPAD_EXT_REG_DPD			= 0xb0,
};

// https://wiibrew.org/wiki/Wiimote#EEPROM_Memory
typedef struct WPADGameInfo
{
	OSTime		timestamp;			// size 0x08, offset 0x00
	u16			gameTitle[16 + 1];	// size 0x22, offset 0x08
	char		gameCode[4];		// size 0x04, offset 0x2a
	OSAppType	gameType;			// size 0x01, offset 0x2e
	byte1_t		checksum;			// size 0x01, offset 0x2f

	/* wiibrew says this exists in the header on the Wiimote but goes unused,
	 * which matches up with the code I see here
	 */
	byte_t		_pad0[8];
} WPADGameInfo; // size 0x38

/*******************************************************************************
 * functions
 */

WPADResult WPADWriteGameData(WPADChannel chan, void const *data, u16 len,
                             u16 addr, WPADCallback *cb);
WPADResult WPADWriteFaceData(WPADChannel chan, void const *data, u16 len,
                             u16 addr, WPADCallback *cb);
WPADResult WPADReadGameData(WPADChannel chan, void *data, u16 len, u16 addr,
                            WPADCallback *cb);
WPADResult WPADReadFaceData(WPADChannel chan, void *data, u16 len, u16 addr,
                            WPADCallback *cb);
WPADResult WPADReadMemoryAsync(WPADChannel chan, void *data, u16 len, u32 addr,
                               WPADCallback *cb);
WPADResult WPADWriteMemoryAsync(WPADChannel chan, void const *data, u16 len,
                                u32 addr, WPADCallback *cb);
WPADResult WPADGetGameDataTimeStamp(WPADChannel chan, OSTime *time);
WPADResult WPADGetGameTitleUtf16(WPADChannel chan, u16 **title);
void WPADSetGameTitleUtf16(u16 const *title);
WPADResult WPADWriteExtReg(WPADChannel chan, void const *data, u16 len,
                           WPADExtRegType extReg, u16 addr, WPADCallback *cb);
WPADResult WPADReadExtReg(WPADChannel chan, void *data, u16 len,
                          WPADExtRegType extReg, u16 addr, WPADCallback *cb);

#ifdef __cplusplus
	}
#endif

#endif // RVL_SDK_WPAD_MEMORY_H
