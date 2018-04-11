#ifndef _KEY_LED_TABLE_H_
#define _KEY_LED_TABLE_H_
#include "stm32f10x_conf.h"
#include "user_conf.h"

#define LOC(x, y) 		((((x - 1) & 0xFF) << 8) | ((y - 1) & 0xFF))  	/* 高8 位X 的位置，低8 位Y 的位置 */
#define GET_X(loc)		((loc >> 8) & 0xFF)
#define GET_Y(loc)		(loc & 0xFF)
#define GET_XY(loc) 	GET_X(loc), GET_Y(loc)

extern u8 g_u8KeyTable[KEY_Y_CNT][KEY_X_CNT];
enum 
{
	_Key_Switch_1 = 1,
	_Key_Switch_2,
	
	_Key_Ctrl_Reserved,

};


enum 
{
	_Led_Record_Record = LOC(1, 2),
	_Led_Record_Pause = LOC(2, 2),
	_Led_Record_Stop = LOC(3, 2),
	
	_Led_Fun_Reserved1 = LOC(4, 2),
	_Led_Fun_Reserved2 = LOC(5, 2),
	_Led_Fun_Reserved3 = LOC(6, 2),
	
	_Led_Fun_CG1 = LOC(1, 3),
	_Led_Fun_CG2 = LOC(2, 3),
	_Led_Fun_CG3 = LOC(3, 3),
	_Led_Fun_CG4 = LOC(4, 3),
	_Led_Fun_CG5 = LOC(5, 3),
	_Led_Fun_CG6 = LOC(6, 3),


	_Led_PGM_1 = LOC(1, 6),
	_Led_PGM_2 = LOC(2, 6),
	_Led_PGM_3 = LOC(3, 6),
	_Led_PGM_4 = LOC(4, 6),
	_Led_PGM_5 = LOC(5, 6),
	_Led_PGM_6 = LOC(6, 6),
	_Led_PGM_7 = LOC(7, 6),
	_Led_PGM_8 = LOC(8, 6),
	_Led_PGM_9 = LOC(1, 4),
	_Led_PGM_10 = LOC(2, 4),
	_Led_PGM_11 = LOC(3, 4),
	_Led_PGM_12 = LOC(4, 4),


	_Led_PVW_1 = LOC(1, 5),
	_Led_PVW_2 = LOC(2, 5),
	_Led_PVW_3 = LOC(3, 5),
	_Led_PVW_4 = LOC(4, 5),
	_Led_PVW_5 = LOC(5, 5),
	_Led_PVW_6 = LOC(6, 5),
	_Led_PVW_7 = LOC(7, 5),
	_Led_PVW_8 = LOC(8, 5),
	_Led_PVW_9 = LOC(1, 1),
	_Led_PVW_10 = LOC(2, 1),
	_Led_PVW_11 = LOC(3, 1),
	_Led_PVW_12 = LOC(4, 1),	

	_Led_Cam_1 = LOC(1, 8),
	_Led_Cam_2 = LOC(1, 7),
	_Led_Cam_3 = LOC(2, 8),
	_Led_Cam_4 = LOC(2, 7),
	
	_Led_Cam_Ctrl_Tele = LOC(3, 8),
	_Led_Cam_Ctrl_Wide = LOC(3, 7),

	_Led_Effect_1 = LOC(1, 9),
	_Led_Effect_2 = LOC(1, 10),
	_Led_Effect_3 = LOC(2, 9),
	_Led_Effect_4 = LOC(2, 10),
	_Led_Effect_5 = LOC(3, 9),
	_Led_Effect_6 = LOC(3, 10),
	
	_Led_Effect_Ctrl_Take = LOC(4, 10),
	_Led_Effect_Ctrl_Cut = LOC(4, 9),
	
};

#endif

