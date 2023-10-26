#ifndef __MOD_INPUT_H
#define __MOD_INPUT_H

#include <rtthread.h>
#include "utils_list.h"

#define DEV_TYPE_ENCODER	0x22
#define DEV_TYPE_TS			0x12
#define DEV_TYPE_KEYPAD		0x32

typedef struct _InputOpr {
	uint8_t id;          		/* 输入模块的id */
    rt_device_t InputDev;
	struct rt_semaphore IrqSem;
	void (*event_proc)(struct _InputOpr *);  	/* 设备定时或中断需要执行的任务 */
	void *user_data;
}M_InputOpr, *PM_InputOpr;


extern M_listNode *Input_DevFind(M_listObject *ptList, uint8_t id);
extern int InputDevInit(M_listObject *InputDev_list, uint8_t id, void *obj);


#if defined(RT_USING_ENCODER_INPUTDEV)
	extern int HW_EncoderInputRegister(M_listObject *list, void *obj);
#endif

#if defined(RT_USING_TS_INPUTDEV)
	extern int HW_TsInputRegister(M_listObject *list, void *obj);
#endif

#if defined(RT_USING_KEYPAD_INPUTDEV)
	extern int HW_KeyInputRegister(M_listObject *list, void *obj);
#endif


#endif

