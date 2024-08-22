#ifndef __LV_PORT_INDEV_H
#define __LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#if defined(RT_USING_TS_INPUTDEV)
extern lv_indev_t* ts_indev_obj;
#endif

#if defined(RT_USING_ENCODER_INPUTDEV)
extern lv_indev_t* enc_indev_obj;
#endif

#if defined(RT_USING_KEYPAD_INPUTDEV)
extern lv_indev_t* key_indev_obj;
#endif

extern void lv_port_indev_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

