#ifndef _G9147_H_
#define _G9147_H_
#include <stdint.h>
#include <stdbool.h>

#define GT9147_MAX_TOUCH 5	
typedef struct _tagStPoint
{
	uint16_t u16X;
	uint16_t u16Y;
}StPoint;

bool GT9147Init(void);
bool GT9147Scan(StPoint stPoint[GT9147_MAX_TOUCH], uint8_t *pCnt);

#endif
