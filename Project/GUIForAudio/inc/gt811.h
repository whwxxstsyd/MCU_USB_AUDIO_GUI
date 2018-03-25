#ifndef _G811_H_
#define _G811_H_
#include <stdint.h>
#include <stdbool.h>

#define POINT_CNT 5	
typedef struct _tagStPoint
{
	uint16_t u16X;
	uint16_t u16Y;
}StPoint;

bool GT811Init(void);
bool GT811Scan(StPoint stPoint[POINT_CNT], uint8_t *pCnt);

#endif
