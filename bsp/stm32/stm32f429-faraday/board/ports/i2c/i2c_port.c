/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-4-06     wzlxy         The first version for STM32F4xx
 */

#include <board.h>
#include "gt9147.h"
#include "pcf8574.h"


#if defined(BSP_USING_I2C1)


#define GT9147_RST_PIN    GET_PIN(I, 8)
#define GT9147_IRQ_PIN    GET_PIN(H, 7)

int rt_hw_gt9147_port(void)
{
    struct rt_touch_config config;
    rt_uint8_t rst;
	
    rst = GT9147_RST_PIN;
    config.dev_name = "i2c1";
    config.irq_pin.pin  = GT9147_IRQ_PIN;
    config.irq_pin.mode = PIN_MODE_INPUT_PULLDOWN;
    config.user_data = &rst;
    rt_hw_gt9147_init("touchPad", &config);
    return 0;
}
INIT_ENV_EXPORT(rt_hw_gt9147_port);
#endif



#if defined(BSP_USING_I2C2)

#define ENCODER_IRQ_PIN    GET_PIN(C, 4)
#define PCF8574_IRQ_PIN    GET_PIN(C, 1)

int rt_hw_pcf8574_port(void)
{
    struct rt_touch_config config;
    rt_uint8_t encoder_irq;
	
    encoder_irq = ENCODER_IRQ_PIN;
    config.dev_name = "i2c2";
    config.irq_pin.pin  = PCF8574_IRQ_PIN;
    config.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
    config.user_data = &encoder_irq;
    rt_hw_pcf8574_init("KeyPad", &config, PCF8574_ADDR_DEFAULT);
    return 0;
}
INIT_ENV_EXPORT(rt_hw_pcf8574_port);
#endif


