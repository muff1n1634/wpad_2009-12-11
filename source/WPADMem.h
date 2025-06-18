#ifndef RVL_SDK_WPAD_INTERNAL_MEMORY_H
#define RVL_SDK_WPAD_INTERNAL_MEMORY_H

#include <revolution/WPAD/WPADMem.h>

/*******************************************************************************
 * headers
 */

#include <types.h>

#include <revolution/WPAD/WPAD.h>

/*******************************************************************************
 * types
 */

#ifdef __cplusplus
	extern "C" {
#endif

struct WPADMemBlock
{
	BOOL			busy;			// size 0x04, offset 0x00
	byte_t			const *data;	// size 0x04, offset 0x04
	u16				len;			// size 0x02, offset 0x08
	// 2 bytes padding (probably alignment, see WPADiClearMemBlock)
	u32				addr;			// size 0x04, offset 0x0c
	WPADCallback	*cb;			// size 0x04, offset 0x10
}; // size 0x14

/*******************************************************************************
 * functions
 */

void WPADiClearMemBlock(WPADChannel chan);

#ifdef __cplusplus
	}
#endif

#endif // RVL_SDK_WPAD_INTERNAL_MEMORY_H
