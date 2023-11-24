#ifndef __LV_PORT_INDEV_H
#define __LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_indev_t* ts_indev_obj;

extern void lv_port_indev_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

