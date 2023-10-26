#ifndef __INPUT_KEYPAD_H
#define __INPUT_KEYPAD_H

#ifdef RT_USING_KEYPAD_INPUTDEV

#define KEY_DEV_NAME 	"KeyPad"
#define KEY_DEV_ID      3


typedef struct _KeyPadPara {
	uint8_t last_btn;  
    rt_bool_t bIsPress;
}M_KeyPadPara;


#endif

#endif

