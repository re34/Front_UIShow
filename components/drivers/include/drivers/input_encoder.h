#ifndef __INPUT_ENCODER_H
#define __INPUT_ENCODER_H

#ifdef RT_USING_ENCODER_INPUTDEV

#define PULSE_ENCODER_DEV_NAME    "pulse2"

#define ENCODER_DEV_ID    1

#define ENCODER_BTN_ENTER   2 
#define ENCODER_TRUN_LEFT	1
#define ENCODER_TRUN_RIGHT  0   

typedef enum
{
	monitor_use_Irq = 0,
	monitor_use_timerChk,
} E_Monitor_Mode;


typedef struct _EncoderPara {
	uint8_t direction;		//旋钮左旋或右旋 
    rt_int32_t pulseRelCnt;	    //旋钮空闲计数
    rt_bool_t bIsSendLockSignal;		//是否发送锁定信号
}M_EncoderPara;

#endif

#endif

