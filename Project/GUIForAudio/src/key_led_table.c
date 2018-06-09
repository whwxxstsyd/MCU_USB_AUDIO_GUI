#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "key_led.h"
#include "key_led_table.h"

u8 g_u8KeyTable[KEY_Y_CNT][KEY_X_CNT] = 
{
	{
		0,
		0,

	},	/* 1 */
	{
		0,
		0,

	},	/* 2 */

	{
		0,
		0,
		_Key_Switch_2,

	},	/* 3 */
	{
		0,
		0,
		_Key_Switch_1,
	},	/* 4 */

};

/* dp, g, f, e, d, c, b, a */
const u8 g_u8LED7Code[] = 
{
	0x3F,		// 0
	0x06,		// 1
	0x5B,		// 2
	0x4F,		// 3
	0x66,		// 4
	0x6D,		// 5
	0x7D,		// 6
	0x07,		// 7
	0x7F,		// 8
	0x6F,		// 9
	0x77,		// A
	0x7C,		// B
	0x39,		// C
	0x5E,		// D
	0x79,		// E
	0x71,		// F
	0x40,		// -
};
 

const u16 g_u16CamAddrLoc[CAM_ADDR_MAX] = 
{
	0,
};

