#include <revolution/WPAD/WPADMem.h>
#include "WPADMem.h"

/*******************************************************************************
 * headers
 */

#include <stddef.h> // offsetof
#include <string.h>

#include <macros.h>
#include <types.h>

#include "WPAD.h"

#if 0
#include <revolution/OS/OSInterrupt.h>
#endif

#include <context_rvl.h>

/*******************************************************************************
 * macros
 */

#define MAX_BLOCK_LENGTH	0x10	// that can be transferred at once

#define WPAD_USERAREA_LO	0x002a	/* name known from asserts */
#define WPAD_USERAREA_HI	0x16d1	/* name known from asserts */

#define WPAD_FACEAREA_LO	0x0fca

/*******************************************************************************
 * local function declarations
 */

static void __wpadResultCallback(WPADChannel chan, WPADResult result);
static void __wpadWriteCallback(WPADChannel chan, WPADResult result);
static WPADResult __wpadSendWriteMemory(WPADChannel chan, u16 len);
static void __wpadWriteGameDataSub(WPADChannel chan, WPADResult result);
static void __wpadWriteCheck1(WPADChannel chan, WPADResult result);
static void __wpadWriteCheck2(WPADChannel chan, WPADResult result);

/*******************************************************************************
 * variables
 */

// .bss
static u16 _wpadGameTitle[16 + 1]; // wchar_t

/*******************************************************************************
 * functions
 */

void WPADiClearMemBlock(WPADChannel chan)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	p_wpd->memBlock.busy	= FALSE;
	p_wpd->memBlock.data	= NULL;
	p_wpd->memBlock.len		= 0;
	p_wpd->memBlock.addr	= 0;
	p_wpd->memBlock.cb		= NULL;
}

static void __wpadResultCallback(WPADChannel chan, WPADResult result)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	p_wpd->memBlock.busy = FALSE;

	WPADCallback *cb = p_wpd->memBlock.cb;

	p_wpd->memBlock.cb = NULL;

	if (cb)
		(*cb)(chan, result);
}

static void __wpadWriteCallback(WPADChannel chan, WPADResult result_)
{
	WPADResult result = result_; // mhm. Yes

	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	if (!p_wpd->memBlock.len)
	{
		__wpadResultCallback(chan, result);
		return;
	}

	u16 blockLen = MIN(MAX_BLOCK_LENGTH, p_wpd->memBlock.len);

	if (result == WPAD_ESUCCESS)
	{
		p_wpd->memBlock.len -= blockLen;
		p_wpd->memBlock.addr += MAX_BLOCK_LENGTH;
		p_wpd->memBlock.data += MAX_BLOCK_LENGTH;

		__wpadSendWriteMemory(chan, blockLen);
	}
	else if (result == WPAD_EBUSY)
	{
		__wpadSendWriteMemory(chan, blockLen);
	}
	else
	{
		__wpadResultCallback(chan, result);
	}
}

static WPADResult __wpadSendWriteMemory(WPADChannel chan, u16 len)
{
	WPADResult result;

	BOOL intrStatus;

	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	intrStatus = OSDisableInterrupts();

	result = WPADWriteMemoryAsync(chan, p_wpd->memBlock.data, len,
	                              p_wpd->memBlock.addr, &__wpadWriteCallback);

	OSRestoreInterrupts(intrStatus);

	return result;
}

static void __wpadWriteGameDataSub(WPADChannel chan, WPADResult result_)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	WPADResult result = result_;

	u16 blockLen;

	if (p_wpd->unk_0x038[1] == 0)
		p_wpd->unk_0x038[1] = result == WPAD_ESUCCESS ? 0 : -3;

	if (result == WPAD_ESUCCESS)
	{
		if (p_wpd->unk_0x038[0] == 0 || p_wpd->unk_0x038[1] == 0)
		{
			blockLen = MIN(MAX_BLOCK_LENGTH, p_wpd->memBlock.len);

			p_wpd->memBlock.len -= blockLen;

			__wpadSendWriteMemory(chan, blockLen);
			return;
		}

		result = WPAD_ETRANSFER;
	}

	if (p_wpd->memBlock.cb)
	{
		p_wpd->memBlock.busy = FALSE;

		(*p_wpd->memBlock.cb)(chan, result);

		p_wpd->memBlock.cb = NULL;
	}
}

static void __wpadWriteCheck1(WPADChannel chan, WPADResult result)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	if (p_wpd->unk_0x038[0] == 0)
		p_wpd->unk_0x038[0] = result == WPAD_ESUCCESS ? 0 : -3;
}

static void __wpadWriteCheck2(WPADChannel chan, WPADResult result)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	if (p_wpd->unk_0x038[1] == 0)
		p_wpd->unk_0x038[1] = result == WPAD_ESUCCESS ? 0 : -3;
}

WPADResult WPADWriteGameData(WPADChannel chan, void const *data, u16 len,
                             u16 addr, WPADCallback *cb)
{
	u16 offset = sizeof(WPADGameInfo);

	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	BOOL intrStatus;

	BOOL handshakeFinished;
	byte_t *asBytes; // Thanks. For being a bitch

	WPADResult result;

	// TODO: what address is this?
	OSAssertMessage_Line(315, len + addr <= 0xf30,
	                     "Error: Write data into out of boundary\n");

	intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	handshakeFinished = p_wpd->handshakeFinished;

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	WPADGameInfo *gameInfo;
	u8 i;
	u32 addrBase;
	if (!p_wpd->memBlock.busy)
	{
		if (!WPADiIsAvailableCmdQueue(&p_wpd->stdCmdQueue, 9))
		{
			result = WPAD_EBUSY;
			goto end;
		}

		p_wpd->memBlock.busy	= TRUE;
		p_wpd->memBlock.cb		= cb;
		p_wpd->memBlock.len		= len;
		p_wpd->memBlock.addr	= WPAD_USERAREA_LO + offset * 2 + addr;
		p_wpd->memBlock.data	= data;

		gameInfo = &p_wpd->gameInfo;

		memcpy(gameInfo->gameCode, WPADiGetGameCode(),
		       sizeof gameInfo->gameCode);
		memcpy(gameInfo->gameTitle, _wpadGameTitle,
		       sizeof gameInfo->gameTitle);

		gameInfo->timestamp	= OSGetTime();
		gameInfo->gameType	= WPADiGetGameType();
		gameInfo->checksum	= 0x00;

		asBytes = (byte_t *)gameInfo;

		for (i = 0; i < offsetof(WPADGameInfo, checksum); ++i)
			gameInfo->checksum += asBytes[i];

		gameInfo->checksum += 0x55;

		p_wpd->unk_0x038[0] = 0;
		p_wpd->unk_0x038[1] = 0;

		addrBase = WPAD_USERAREA_LO;

		// writing in chunks of 0x10 and 0x08
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x00, 0x10,
		                     addrBase + 0x00, &__wpadWriteCheck1);
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x10, 0x10,
		                     addrBase + 0x10, &__wpadWriteCheck1);
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x20, 0x10,
		                     addrBase + 0x20, &__wpadWriteCheck1);
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x30, 0x08,
		                     addrBase + 0x30, &__wpadWriteCheck1);

		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x00, 0x10,
		                     addrBase + 0x30 + 0x08, &__wpadWriteCheck2);
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x10, 0x10,
		                     addrBase + 0x30 + 0x18, &__wpadWriteCheck2);
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x20, 0x10,
		                     addrBase + 0x30 + 0x28, &__wpadWriteCheck2);
		WPADWriteMemoryAsync(chan, (byte_t *)gameInfo + 0x30, 0x08,
		                     addrBase + 0x30 + 0x38, &__wpadWriteGameDataSub);

		OSRestoreInterrupts(intrStatus);

		return WPAD_ESUCCESS;
	}
	else
	{
		result = WPAD_EBUSY;
	}

end:
	OSRestoreInterrupts(intrStatus);

	if (cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADWriteFaceData(WPADChannel chan, void const *data, u16 len,
                             u16 addr, WPADCallback *cb)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	WPADResult result = WPAD_ESUCCESS;

	BOOL intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	BOOL handshakeFinished = p_wpd->handshakeFinished;

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	u16 blockLen;
	if (!p_wpd->memBlock.busy)
	{
		blockLen = MIN(MAX_BLOCK_LENGTH, len);

		p_wpd->memBlock.busy	= TRUE;
		p_wpd->memBlock.cb		= cb;
		p_wpd->memBlock.addr	= WPAD_FACEAREA_LO + addr;
		p_wpd->memBlock.data	= data;
		p_wpd->memBlock.len		= len - blockLen;

		OSRestoreInterrupts(intrStatus);

		return __wpadSendWriteMemory(chan, blockLen);
	}
	else
	{
		result = WPAD_EBUSY;
	}

end:
	OSRestoreInterrupts(intrStatus);

	if (cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADReadGameData(WPADChannel chan, void *data, u16 len, u16 addr,
                            WPADCallback *cb)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	WPADResult result;

	BOOL intrStatus;

	u32 addrBase; // You too
	BOOL handshakeFinished;

	intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	handshakeFinished = p_wpd->handshakeFinished;

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	if (!p_wpd->memBlock.busy)
	{
		result = p_wpd->unk_0x038[0] == 0 || p_wpd->unk_0x038[1] == 0
		           ? WPAD_CESUCCESS
		           : WPAD_CE6;

		if (result == WPAD_ESUCCESS)
		{
			if (memcmp(p_wpd->gameInfo.gameCode, WPADiGetGameCode(),
			           sizeof p_wpd->gameInfo.gameCode)
			    == 0)
			{
				p_wpd->memBlock.busy	= TRUE;
				p_wpd->memBlock.cb		= cb;

				// TODO: what address is this?
				addrBase = 0x9a;

				OSRestoreInterrupts(intrStatus);

				return WPADReadMemoryAsync(chan, data, len, addrBase + addr,
				                           &__wpadResultCallback);
			}
			else
			{
				result = WPAD_ERR_5;
			}
		}
	}
	else
	{
		result = WPAD_EBUSY;
	}

end:
	OSRestoreInterrupts(intrStatus);

	if (cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADReadFaceData(WPADChannel chan, void *data, u16 len,
                            u16 addr, WPADCallback *cb)
{
	WPADResult result;

	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	BOOL intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	BOOL handshakeFinished = p_wpd->handshakeFinished;

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	if (!p_wpd->memBlock.busy)
	{
		p_wpd->memBlock.busy	= TRUE;
		p_wpd->memBlock.cb		= cb;

		OSRestoreInterrupts(intrStatus);

		return WPADReadMemoryAsync(chan, data, len, WPAD_FACEAREA_LO + addr,
		                           &__wpadResultCallback);
	}
	else
	{
		result = WPAD_EBUSY;
	}

end:
	OSRestoreInterrupts(intrStatus);

	if (cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADReadMemoryAsync(WPADChannel chan, void *data, u16 len,
                                u32 addr, WPADCallback *cb)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	BOOL success;

	BOOL intrStatus;
	BOOL handshakeFinished;

	WPADResult result = WPAD_ESUCCESS;

	OSAssert_Line(605, WPAD_USERAREA_LO <= addr
		&& (u32)(addr + (u32)len) <= WPAD_USERAREA_HI + 1);

	intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	handshakeFinished = p_wpd->handshakeFinished;

	OSRestoreInterrupts(intrStatus);

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	success = WPADiSendReadData(&p_wpd->stdCmdQueue, data, len, addr, cb);

	result = success ? WPAD_CESUCCESS : WPAD_CEBUSY;

end:
	if (result != WPAD_ESUCCESS && cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADWriteMemoryAsync(WPADChannel chan, void const *data, u16 len,
                                u32 addr, WPADCallback *cb)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	BOOL success;

	BOOL intrStatus;
	BOOL handshakeFinished;

	WPADResult result = WPAD_ESUCCESS;

	OSAssert_Line(658, WPAD_USERAREA_LO <= addr
		&& (u32)(addr + (u32)len) <= WPAD_USERAREA_HI + 1);

	intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	handshakeFinished = p_wpd->handshakeFinished;

	OSRestoreInterrupts(intrStatus);

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	success = WPADiSendWriteData(&p_wpd->stdCmdQueue, data, len, addr, cb);
	result = success ? WPAD_CESUCCESS : WPAD_CEBUSY;

end:
	if (result != WPAD_ESUCCESS && cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADGetGameDataTimeStamp(WPADChannel chan, OSTime *time)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	WPADResult result;

	BOOL intrStatus = OSDisableInterrupts();

	result = p_wpd->unk_0x038[0] == 0 || p_wpd->unk_0x038[1] == 0 ? WPAD_CESUCCESS
	                                                            : WPAD_CEINVAL;

	if (result == WPAD_ESUCCESS)
		*time = p_wpd->gameInfo.timestamp;

	OSRestoreInterrupts(intrStatus);

	return result;
}

WPADResult WPADGetGameTitleUtf16(WPADChannel chan, u16 **title)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	WPADResult result;

	BOOL intrStatus = OSDisableInterrupts();

	result = p_wpd->unk_0x038[0] == 0 || p_wpd->unk_0x038[1] == 0 ? WPAD_CESUCCESS
	                                                            : WPAD_CEINVAL;

	if (result == WPAD_ESUCCESS)
		*title = p_wpd->gameInfo.gameTitle;
	else
		*title = NULL;

	OSRestoreInterrupts(intrStatus);

	return result;
}

void WPADSetGameTitleUtf16(u16 const *title)
{
	// Hello. Have you heard of strncpy

	int i;
	for (i = 0; i < (int)ARRAY_LENGTH(_wpadGameTitle) - 1; ++i)
	{
		_wpadGameTitle[i] = title[i];

		if (title[i] == L'\0')
			break;
	}

	_wpadGameTitle[16] = L'\0';
}

WPADResult WPADWriteExtReg(WPADChannel chan, void const *data, u16 len,
                           WPADExtRegType extReg, u16 addr, WPADCallback *cb)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	BOOL success;
	BOOL intrStatus;
	BOOL handshakeFinished;

	WPADResult result = WPAD_ESUCCESS;

	intrStatus = OSDisableInterrupts();

	result = p_wpd->status;

	handshakeFinished = p_wpd->handshakeFinished;

	OSRestoreInterrupts(intrStatus);

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	u32 finalAddress = (addr & 0xffff) | (extReg << 16) | (1 << 26);

	success =
		WPADiSendWriteData(&p_wpd->stdCmdQueue, data, len, finalAddress, cb);
	result = success ? WPAD_CESUCCESS : WPAD_CEBUSY;

end:
	if (result != WPAD_ESUCCESS && cb)
		(*cb)(chan, result);

	return result;
}

WPADResult WPADReadExtReg(WPADChannel chan, void *data, u16 len,
                          WPADExtRegType extReg, u16 addr, WPADCallback *cb)
{
	wpad_cb_st *p_wpd = __rvl_p_wpadcb[chan];

	BOOL success;
	BOOL intrStatus;
	BOOL handshakeFinished;

	WPADResult result = WPAD_ESUCCESS;

	intrStatus = OSDisableInterrupts();

	result = p_wpd->status;
	handshakeFinished = p_wpd->handshakeFinished;

	OSRestoreInterrupts(intrStatus);

	if (result == WPAD_ENODEV)
		goto end;

	if (!handshakeFinished)
	{
		result = WPAD_EBUSY;
		goto end;
	}

	u32 finalAddress = (addr & 0xffff) | (extReg << 16) | (1 << 26);

	success =
		WPADiSendReadData(&p_wpd->stdCmdQueue, data, len, finalAddress, cb);
	result = success ? WPAD_CESUCCESS : WPAD_CEBUSY;

end:
	if (result != WPAD_ESUCCESS && cb)
		(*cb)(chan, result);

	return result;
}
