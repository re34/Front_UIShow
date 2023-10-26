/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-4-06     wzlxy         The first version for STM32F4xx
 */

/************************************************************************************
*刷新率       = 9MHz /((Width + HSYNC_W  + HBP  + HFP)*(Height + VSYNC_W +  VBP  + VFP))
*          = 9000000/((480 + 1 + 40 + 5)*(272 + 1 +  8 + 8))*
*          = 9000000/(916*502)
*          = 59HZ  
*
* Horizontal Synchronization (Hsync)  对应变量HSYNC_W
* Horizontal Back Porch (HBP)         对应变量HBP
* Active Width                        对应变量Width
* Horizontal Front Porch (HFP)        对应变量HFP
* Vertical Synchronization (Vsync)    对应变量VSYNC_W
* Vertical Back Porch (VBP)           对应变量VBP
* Active Heigh                        对应变量Heigh
* Vertical Front Porch (VFP)          对应变量VFP
*
*  LCD_TFT 窗口水平和垂直的起始以及结束位置 :
*     ----------------------------------------------------------------
*   
*     HorizontalStart = (Offset_X + Hsync + HBP);
*     HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
*     VarticalStart   = (Offset_Y + Vsync + VBP);
*     VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);

************************************************************************************/

#ifndef __LCD_PORT_H__
#define __LCD_PORT_H__

#define LCD_WIDTH              480
#define LCD_HEIGHT             272
#define LCD_BITS_PER_PIXEL     16
#define LCD_BUF_SIZE           (LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8)
#define LCD_PIXEL_FORMAT       RTGRAPHIC_PIXEL_FORMAT_RGB565


#define LCD_HSYNC_WIDTH    1
#define LCD_VSYNC_HEIGHT   1

#define LCD_HBP			40
#define LCD_HFP     5

#define LCD_VBP			8
#define LCD_VFP     8

/***************************
*背光配置  ，前提：使能LCD_BACKLIGHT_USING_GPIO
**************************/
#define LCD_BACKLIGHT_USING_GPIO

#define LCD_BL_GPIO_NUM   GET_PIN(B, 5)


#endif
