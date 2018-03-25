/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：user_conf.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _USER_CONF_H_
#define _USER_CONF_H_
#include <stdint.h>

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_KEEP		0xEE
#define KEY_MOVE		0xDD

#define KEY_MIX_MAX				8				/* 最大组合键数量 */




#define MSG_BUF_MAX					8	/* 消息缓冲数量 */

#define BUF_CHECK_SIZE				32	/* 消息过滤 buffer 大小 */

#define KEY_BUF_MAX					8	/* 键盘消息缓冲数量 */


//#define SYNC_MAX_TIME	(KEY_SCAN_CNT * 2 * KEY_BUF_MAX) /* 最大同步时间 单位 ms */
#define SYNC_MAX_TIME	(10 * 1000)	/* 10s */


#define DATA_SAVE_ADDR  		(0x0807D000)
#define AUDIO_DATA_SAVE_ADDR	(0x0807E000)
#define DATA_SAVE_HEAD			(0xA5A5)

typedef struct _tagStKeyState
{
	uint8_t u8KeyValue;		/* 值 */
	uint8_t u8KeyLocation;	/* 位置/ ID */
	uint8_t u8KeyState;		/* 状态 */
	uint8_t u8Reserved;
}StKeyState;


#endif

