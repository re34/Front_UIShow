/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-21     SummerGift   first version
 * 2018-11-22     flybreak     Make the first version of pcf8574's package
 */

#ifndef __PCF8574_H
#define __PCF8574_H

#include <rtthread.h>
#include "touch.h"

#define PCF8574_ADDR_DEFAULT    0x20
#define READ_ALL_BIT	8
#define ENCODER_IRQ_OCCUR  0x66


int rt_hw_pcf8574_init(const char *dev_name, struct rt_touch_config *cfg, rt_uint8_t i2c_addr);
void rt_hw_pcf8574_deinit(struct rt_touch_device *dev);

#endif

