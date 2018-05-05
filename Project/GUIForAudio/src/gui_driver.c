#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "lvgl.h"

#include "user_init.h"
#include "user_api.h"
#include "lcd.h"
#include "I2C.h"
#include "gt9147.h"
#include "gui_driver.h"
#include "gui.h"

#if USE_LVGL

#if 1
/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
#if LV_VDB_DOUBLE == 0
	int32_t s32Length = (x2 - x1 + 1) * (y2 - y1 + 1);

	LCDSetCursor(x1, y1);	//设置光标位置 
	LCDSetXEnd(x2);
	LCDWriteRAMPrepare();     //开始写入GRAM	 	  
	LCDDMAWrite((const uint16_t *)color_p, s32Length);	
	lv_flush_begin();

	/* IMPORTANT!!!
	 * Inform the graphics library that you are ready with the flushing*/
	lv_flush_ready();
	
#else
#define LCD_DMA					DMA1_Channel2
#define LCD_DMA_FINISH_FLAG		DMA1_FLAG_TC2

	static bool boIsFlushing = false;
	int32_t s32Length;
	
	if (boIsFlushing)
	{
		/* Check if DMA channel transfer is finished */
		while(!DMA_GetFlagStatus(LCD_DMA_FINISH_FLAG));

		/* Clear DMA channel transfer complete flag bit */
		DMA_ClearFlag(LCD_DMA_FINISH_FLAG);

		/* disable */
		LCD_DMA->CCR = 0;
		
		boIsFlushing = false;
		
		/* IMPORTANT!!!
		 * Inform the graphics library that you are ready with the flushing*/
		lv_flush_ready();
		
	}


	s32Length = (x2 - x1 + 1) * (y2 - y1 + 1);

	LCDSetCursor(x1, y1);	//设置光标位置 
	LCDSetXEnd(x2);
	LCDWriteRAMPrepare();     //开始写入GRAM	 	  
	//LCDDMAWrite((const uint16_t *)color_p, s32Length);
	{
		
		LCD_DMA->CPAR = (u32)color_p;
		LCD_DMA->CMAR = (u32)(&(LCD->LCDRam));
		
		LCD_DMA->CNDTR = s32Length;
		
		LCD_DMA->CCR = DMA_PeripheralInc_Enable | DMA_PeripheralDataSize_HalfWord |
						DMA_MemoryDataSize_HalfWord | DMA_Priority_High | DMA_M2M_Enable;
		/* enable */
		LCD_DMA->CCR |= DMA_CCR1_EN;
		

	}
	
	boIsFlushing = true;
	
	lv_flush_begin();
#endif
}
#endif

#if LV_VDB_SIZE == 0

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    int32_t x;
    int32_t y;
    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            color_p++;
        }
    }

}


/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2,  lv_color_t color)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    int32_t x;
    int32_t y;
    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color)*/
        }
    }
}
#endif

/* Read the touchpad and store it in 'data'
 * REaturn false if no more data read; true for ready again */
static bool ex_tp_read(lv_indev_data_t *data)
{
    /* Read your touchpad */
    /* data->state = LV_INDEV_STATE_REL or LV_INDEV_STATE_PR */
    /* data->point.x = tp_x; */
    /* data->point.y = tp_y; */
	
	static StPoint stLastPoint[GT9147_MAX_TOUCH] = {0, 0};
	uint32_t u32Index = (uint32_t)(data->user_data);
	
	StPoint stPoint[GT9147_MAX_TOUCH];
	uint8_t u8Cnt = 0;
	GT9147Scan(stPoint, &u8Cnt);
	if (u8Cnt < (u32Index + 1))
	{
		data->point.x = stLastPoint[u32Index].u16X;
		data->point.y = stLastPoint[u32Index].u16Y;
		data->state = LV_INDEV_STATE_REL;					
	}
	else
	{
		data->state = LV_INDEV_STATE_PR;
		data->point.x = stLastPoint[u32Index].u16X = stPoint[u32Index].u16X;
		data->point.y = stLastPoint[u32Index].u16Y = stPoint[u32Index].u16Y;
	}
	

    return false;   /*false: no more data to read because we are no buffering*/
}

static uint32_t last_key = LV_GROUP_KEY_ESC;
static lv_indev_state_t state = LV_INDEV_STATE_REL;

/**
 * Get the last pressed or released character from the PC's keyboard
 * @param data store the read data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool keyboard_read(lv_indev_data_t * data)
{
    data->state = state;
    data->key = last_key;
    return false;
}

void SetLvglKey(uint32_t u32Key, bool boIsPress)
{
	last_key = u32Key;
	state = boIsPress ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}


static void hal_init(void)
{
    /* Add a display */
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.disp_flush = ex_disp_flush;
#if LV_VDB_SIZE == 0
    disp_drv.disp_fill = ex_disp_fill;
    disp_drv.disp_map = ex_disp_map;
#else
    disp_drv.disp_fill = NULL;
    disp_drv.disp_map = NULL;
	
#endif
    lv_disp_drv_register(&disp_drv);

    /* Add the touchpad as input device */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read = ex_tp_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv.user_data = (void *) 0;
	lv_indev_drv_register(&indev_drv);
	
    indev_drv.user_data = (void *) 1;
	lv_indev_drv_register(&indev_drv);
}


void LvglInit(void)
{
    /*Initialize LittlevGL*/
    lv_init();
	hal_init();
}

void LvglDispMem(void)
{
	static lv_obj_t * label = NULL;
	lv_mem_monitor_t stMemMonitor;
	char c8Buf[64];

	lv_mem_monitor(&stMemMonitor);
	sprintf(c8Buf, "free: %d%%, using: %d",
		stMemMonitor.free_size * 100 / stMemMonitor.total_size,
		stMemMonitor.total_size - stMemMonitor.free_size);
	{
		StPoint stPoint[GT9147_MAX_TOUCH];
		uint8_t u8Cnt = 0;
		GT9147Scan(stPoint, &u8Cnt);
		if (u8Cnt != 0)
		{
			sprintf(c8Buf + strlen(c8Buf), "; c: %d, x: %d, y: %d",
				u8Cnt, stPoint[0].u16X, stPoint[0].u16Y);			
		}

	}
	if (label != NULL)
	{
		lv_obj_del(label);
	}
	label = lv_label_create(lv_scr_act(), NULL);
	if (label != NULL)
	{
		lv_label_set_text(label, c8Buf);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -50);
	}

}

void LvglTickInc(void)
{
	lv_tick_inc(1);	
}

void LvglFlushTask(void)
{
	lv_task_handler();
}

#endif
