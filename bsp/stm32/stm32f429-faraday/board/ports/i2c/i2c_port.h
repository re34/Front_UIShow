/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-4-06     wzlxy         The first version for STM32F4xx
 */



#ifndef __I2C_PORT_H__
#define __I2C_PORT_H__

/*****************************
* use for GT9147 touch Chip
******************************/
#ifdef BSP_USING_I2C1

#define BSP_I2C1_SCL_PIN   GET_PIN(H, 6)
#define BSP_I2C1_SDA_PIN   GET_PIN(I, 3)

#endif

#ifdef BSP_USING_I2C2

#define BSP_I2C2_SCL_PIN   GET_PIN(H, 4)
#define BSP_I2C2_SDA_PIN   GET_PIN(H, 5)

#endif



#endif
