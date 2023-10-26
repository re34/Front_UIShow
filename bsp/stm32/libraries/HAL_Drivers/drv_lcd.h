#ifndef __DRV_LCD_H
#define __DRV_LCD_H


#include <board.h>

//#define RT_USING_LTDC_DOUBLE_BUF

struct drv_lcd_device
{
    struct rt_device parent;
    struct rt_device_graphic_info lcd_info;
    struct rt_semaphore lcd_lock;

#ifdef RT_USING_LTDC_DOUBLE_BUF
    /* 0:front_buf is being used 1: back_buf is being used*/
	struct rt_semaphore lcd_dmaSet;
    rt_uint8_t cur_buf;
    rt_uint8_t *front_buf;
    rt_uint8_t *back_buf;
#endif
};

#define LCD_DEVICE(dev)     (struct drv_lcd_device*)(dev)





#endif

