/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：gui.h
* 摘要: 界面控制相关程序头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2018年04月06日
*******************************************************************************/


#ifndef GUI_H_
#define GUI_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "lvgl/lvgl.h"

#define USE_LVGL	1

#ifdef _WIN32
#define __weak
#endif
typedef enum _tagEmAudioCtrlMode
{
	_Audio_Ctrl_Mode_Normal = 0,	/* left to left, right to right */
	_Audio_Ctrl_Mode_ShieldLeft,
	_Audio_Ctrl_Mode_ShieldRight,
	_Audio_Ctrl_Mode_ShieldLeftAndRight,
	_Audio_Ctrl_Mode_LeftToRight,
	_Audio_Ctrl_Mode_RightToLeft,
	_Audio_Ctrl_Mode_Mux,

	_Audio_Ctrl_Mode_Reserved,
}EmAudioCtrlMode;

enum
{
	_Channel_AIN_1,
	_Channel_AIN_2,
	_Channel_AIN_3,
	_Channel_AIN_4,
	_Channel_AIN_5,
	_Channel_AIN_Mux,
	_Channel_PC,
	_Channel_HeaderPhone,
	_Channel_InnerSpeaker,
	_Channel_NormalOut,

	_Channel_Reserved,
};

#ifndef TOTAL_MODE_CTRL_IN
#define TOTAL_MODE_CTRL_IN		7
#endif

#ifndef TOTAL_MODE_CTRL_OUT
#define TOTAL_MODE_CTRL_OUT		3
#endif

#ifndef TOTAL_MODE_CTRL
#define TOTAL_MODE_CTRL			_Channel_Reserved
#endif

#ifndef TOTAL_EXTERN_MODE_CTRL
#define TOTAL_EXTERN_MODE_CTRL		7
#endif

#ifndef TOTAL_VOLUME_CHANNEL
#define TOTAL_VOLUME_CHANNEL 			TOTAL_MODE_CTRL
#endif


#ifndef TOTAL_EXTERN_VOLUME_CHANNEL
#define TOTAL_EXTERN_VOLUME_CHANNEL 			TOTAL_EXTERN_MODE_CTRL
#endif

#ifndef PHANTOM_POWER_CTRL
#define PHANTOM_POWER_CTRL		2
#endif

#ifndef ENABLE_INPUT_CTRL
#define ENABLE_INPUT_CTRL		5
#endif

#ifndef ENABLE_OUTPUT_CTRL
#define ENABLE_OUTPUT_CTRL		3
#endif


typedef struct _tagStVolumeCtrlGroup
{
	lv_obj_t *pLeftVolume;
	lv_obj_t *pRightVolume;
	lv_obj_t *pCtrlMode;
	lv_obj_t *pUniformVolume;


	lv_obj_t *pTipsLabel;
	lv_style_t *pTipsStyle;
	void *pTipsAnim;
	uint16_t u16XPos;

	uint8_t u8Index;
	uint8_t u8MaxCtrlMode;
	const uint8_t *pCtrlModeIndex;

	const char *pTitle;

	bool boIsFixUniformVoume;

}StVolumeCtrlGroup;


typedef struct _tagStVolume
{
	uint8_t u8Channel1;
	uint8_t u8Channel2;
}StVolume;

typedef struct _tagStMemory
{
	StVolume stVolume[TOTAL_VOLUME_CHANNEL];
	EmAudioCtrlMode emAudioCtrlMode[TOTAL_MODE_CTRL];
	bool boPhantomPower[PHANTOM_POWER_CTRL];
	uint8_t u8AINChannelEnableState;
	uint8_t u8OutputChannelEnableState;
}StMemory;

typedef struct _tagStUniformCheckState
{
	bool boUniformCheckState[TOTAL_VOLUME_CHANNEL];
}StUniformCheckState;



typedef struct _tagStMemoryCtrlGroup
{
	lv_obj_t *pMemoryCtrl;
	lv_obj_t *pFactorySet;
	lv_obj_t *pMBox;

	lv_obj_t *pLabel;
	uint8_t u8TmpMemorySelect;


}StMemoryCtrlGroup;

typedef struct _tagStPhantomPowerCtrlGroup
{
	lv_obj_t *pCBArr[PHANTOM_POWER_CTRL];

}StPhantomPowerCtrlGroup;

typedef struct _tagStInputEnableCtrlGroup
{
	lv_obj_t *pCBArr[ENABLE_INPUT_CTRL];

}StInputEnableCtrlGroup;

typedef struct _tagStOutputEnableCtrlGroup
{
	lv_obj_t *pCBArr[ENABLE_OUTPUT_CTRL];

}StOutputEnableCtrlGroup;


enum
{
	_Tab_Input_1_2,
	_Tab_Input_3_5,
	_Tab_Input_PC_Ctrl,
	_Tab_Output,
	_Tab_Other_Ctrl,
	_Tab_SYS_Ctrl,
	
	_Tab_Reserved,
	_Tab_Volume_Show,
	_Tab_Reserved1,


};


enum
{
	_Fun_AudioVolume,
	_Fun_AudioMode,
	_Fun_PhantomPower,
	_Fun_InputEnable,
	_Fun_OutputEnable,

	_Fun_Reserved,
};


int32_t CreateTableView(void);
int32_t ReflushActiveTable(uint32_t u32Fun, uint32_t u32Channel);


int32_t GetAudioCtrlMode(uint16_t u16Channel, EmAudioCtrlMode *pMode);
int32_t SetAudioCtrlMode(uint16_t u16Channel, EmAudioCtrlMode emMode);
int32_t SendAudioCtrlModeCmd(uint16_t u16Channel, EmAudioCtrlMode emMode);
void GetAllAudioCtrlMode(EmAudioCtrlMode emAudioCtrlMode[TOTAL_MODE_CTRL]);
void SetAllAudioCtrlMode(EmAudioCtrlMode emAudioCtrlMode[TOTAL_MODE_CTRL]);


int32_t GetAudioVolume(uint16_t u16Channel, StVolume *pVolume);
int32_t SetAudioVolume(uint16_t u16Channel, StVolume stVolume);
int32_t SendAudioVolumeCmd(uint16_t u16Channel, StVolume stVolume);
void GetAllAudioVolume(StVolume stVolume[TOTAL_VOLUME_CHANNEL]);
void SetAllAudioVolume(StVolume stVolume[TOTAL_VOLUME_CHANNEL]);


int32_t GetPhantomPowerState(uint16_t u16Channel, bool *pState);
int32_t SetPhantomPowerState(uint16_t u16Channel, bool boIsEnable);
int32_t SendPhantomPowerStateCmd(uint16_t u16Channel, bool boIsEnable);
void GetAllPhantomPowerState(bool boState[PHANTOM_POWER_CTRL]);
void SetAllPhantomPowerState(bool boState[PHANTOM_POWER_CTRL]);


uint8_t GetInputEnableState(void);
int32_t SetInputEnableState(uint8_t u8NewState);
int32_t SendInputEnableStateCmd(uint8_t u8NewState);
uint8_t GetOutputEnableState(void);
int32_t SetOutputEnableState(uint8_t u8NewState);
int32_t SendOutputEnableStateCmd(uint8_t u8NewState);


int32_t SendMemeoryCtrlCmd(uint16_t u16Channel, bool boIsSave);
int32_t SendFactoryCtrlCmd(void);

#endif // GUI_H_
