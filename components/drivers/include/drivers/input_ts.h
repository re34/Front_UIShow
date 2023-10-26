#ifndef __INPUT_TS_H
#define __INPUT_TS_H

#ifdef RT_USING_TS_INPUTDEV

#define TS_DEV_NAME    "touchPad"
#define TS_DEV_ID      2


typedef struct _TouchPadPara {
	int16_t last_x;		
    int16_t last_y;	    
    rt_bool_t bIsIrqOccur;
}M_TouchPadPara;


#endif

#endif

