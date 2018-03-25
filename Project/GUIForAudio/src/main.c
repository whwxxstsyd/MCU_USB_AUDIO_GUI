
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "lvgl.h"
//#include "lv_drivers/display/monitor.h"
//#include "lv_drivers/indev/mouse.h"
#include "demo.h"
//#include "lv_examples/lv_tests/lv_test_theme/lv_test_theme.h"
#include "lv_misc/lv_fonts/lv_symbol_def.h"

#include "user_init.h"
#include "user_api.h"
#include "lcd.h"
#include "I2C.h"
#include "gt9147.h"

void LEDInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
}
void LEDToggle(void)
{
	GPIO_WriteBit(GPIOE, GPIO_Pin_5, 
	GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_5) == Bit_SET ? Bit_RESET : Bit_SET);
}


static void hal_init(void);


int main (void)
{
	u32 u32Time = 0;
	//lv_theme_t *pTheme = NULL;
    /*Initialize LittlevGL*/
    lv_init();

    /*Initialize the HAL for LittlevGL*/
	PeripheralClkEnable();
	OpenSpecialGPIO();
	SysTickInit();
	
	
	LCDInit();			//初始化液晶 
	LCDClear(RGB(255, 0, 0));
	LEDInit();
	I2CInit();
	GT9147Init();
	
	hal_init();
    //pTheme = lv_theme_alien_init(10, NULL);

    //lv_test_theme_1(pTheme);

    /*Load a demo*/
    demo_create();
#if 1
	
		/*Create a Label on the currently active screen*/
		lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);

		/*Modify the Label's text*/
		lv_label_set_text(label1, "Hello world!");

		/* Align the Label to the center
		 * NULL means align on parent (which is the screen now)
		 * 0, 0 at the end means an x, y offset after alignment*/
		lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
		
		/*Create a Label on the currently active screen*/
		lv_obj_t * label2 =  lv_label_create(lv_scr_act(), NULL);

		/* Align the Label to the center
		 * NULL means align on parent (which is the screen now)
		 * 0, 0 at the end means an x, y offset after alignment*/
		lv_obj_align(label2, label1, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

	
#endif

    while(1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        Delay(1);       /*Just to let the system breath*/
		if (SysTimeDiff(u32Time, g_u32SysTickCnt) > 500)
		{
			LEDToggle();
			u32Time = g_u32SysTickCnt;
			{
				lv_mem_monitor_t stMemMonitor;
				lv_mem_monitor(&stMemMonitor);
				stMemMonitor.total_size = stMemMonitor.total_size;
				char c8Buf[64];
				sprintf(c8Buf, "Total: %d, free: %d, using: %d%%", 
					stMemMonitor.total_size, stMemMonitor.free_size, stMemMonitor.used_pct);
				lv_label_set_text(label1, c8Buf);
			}
			{
				char c8Buf[64];
				StPoint stPoint[GT9147_MAX_TOUCH];
				uint8_t u8Cnt = 0;
				GT9147Scan(stPoint, &u8Cnt);
				if (u8Cnt == 0)
				{
					sprintf(c8Buf, "Got no point");					
				}
				else
				{
					sprintf(c8Buf, "Got point(%d, %d)", stPoint[0].u16X, stPoint[0].u16Y);
				}
				lv_label_set_text(label2, c8Buf);
			}
		}
    }
}


#if 1
/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
#define LCD_DMA					DMA1_Channel2
#define LCD_DMA_FINISH_FLAG		DMA1_FLAG_TC2

#if 0
    int32_t x;
    int32_t y;
    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            color_p++;
        }
    }
#endif
	int32_t s32Length = (x2 - x1 + 1) * (y2 - y1 + 1);

	LCDSetCursor(x1, y1);	//设置光标位置 
	LCDSetXEnd(x2);
	LCDWriteRAMPrepare();     //开始写入GRAM	 	  
	LCDDMAWrite((const uint16_t *)color_p, s32Length);

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_flush_ready();
}
#endif

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


/* Read the touchpad and store it in 'data'
 * REaturn false if no more data read; true for ready again */
static bool ex_tp_read(lv_indev_data_t *data)
{
    /* Read your touchpad */
    /* data->state = LV_INDEV_STATE_REL or LV_INDEV_STATE_PR */
    /* data->point.x = tp_x; */
    /* data->point.y = tp_y; */
	
	StPoint stPoint[GT9147_MAX_TOUCH];
	uint8_t u8Cnt = 0;
	GT9147Scan(stPoint, &u8Cnt);
	if (u8Cnt == 0)
	{
		data->state = LV_INDEV_STATE_REL;					
	}
	else
	{
		data->state = LV_INDEV_STATE_PR;
		data->point.x = stPoint[0].u16X;
		data->point.y = stPoint[0].u16Y;
	}
	

    return false;   /*false: no more data to read because we are no buffering*/
}

static void hal_init(void)
{
    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    //monitor_init();
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.disp_flush = ex_disp_flush;
    disp_drv.disp_fill = ex_disp_fill;
    disp_drv.disp_map = ex_disp_map;
    lv_disp_drv_register(&disp_drv);

#if 1
    /* Add the touchpad as input device */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read = ex_tp_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_drv_register(&indev_drv);
#endif
}

