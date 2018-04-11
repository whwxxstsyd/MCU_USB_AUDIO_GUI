/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����app_port.c
* ժҪ: ��Ҫ������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��05��08��
*******************************************************************************/
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "app_port.h"

#if CRITICAL_METHOD == 4
__asm void CPU_SetInterruptMask(void)
{
	PRESERVE8

	MOV		R0, #MAX_SYSCALL_INTERRUPT_PRIORITY
	MSR		BASEPRI, R0
	BX		R14
}

__asm void OS_CPU_ClearInterruptMask(void)
{
	PRESERVE8

	/* FAQ:  Setting BASEPRI to 0 is not a bug.  Please see 
	http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html before disagreeing. */
	MOV		R0, #0
	MSR		BASEPRI, R0
	BX		R14
}

#elif CRITICAL_METHOD == 3

__asm int CPU_SR_Save(void)
{
    MRS     R0, PRIMASK                                         
    CPSID   I
    BX      LR
}

__asm void CPU_SR_Restore(int cpu_sr)
{
    MSR     PRIMASK, R0
    BX      LR
}
#endif





