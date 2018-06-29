#ifndef _SCREEN_PROCECT_H_
#define _SCREEN_PROCECT_H_
#include <stdbool.h>
#include <stdint.h>

typedef struct _tagStScreenProtect
{
	uint32_t u32LastKeyTime;

	bool boCurState;
	uint8_t u8ForceProtect;

	uint8_t u8ProtectMode;
	uint8_t u8ProtectTimeIndex;
}StScreenProtect;


enum
{
	_ScreenProtect_15S,
	_ScreenProtect_30,
	_ScreenProtect_1M,
	_ScreenProtect_2M,
	_ScreenProtect_5M,
	_ScreenProtect_10M,
	_ScreenProtect_Close,

	_ScreenProtect_Reserved,
};

enum
{
	_ScreenProtect_Mode_Logo,
	_ScreenProtect_Mode_Close,

	_ScreenProtect_Mode_Reserved,
};

enum
{
	_ScreenProtect_Force_Start = 1,
	_ScreenProtect_Force_Stop,

	_ScreenProtect_Force_Reserved,
};

void SrceenProtectInit(void);
bool SrceenProtectIsStart(void);
void SrceenProtectForceStart(void);
void SrceenProtectForceStop(void);
void SrceenProtectReset(void);
void SrceenProtectFlush(void);

int32_t SrceenProtectSetTime(uint8_t u8TimeIndex);
int32_t SrceenProtectSetMode(uint8_t u8ModeIndex);
uint8_t SrceenProtectGetTime(void);
uint8_t SrceenProtectGetMode(void);


#endif // _SCREEN_PROCECT_H_



