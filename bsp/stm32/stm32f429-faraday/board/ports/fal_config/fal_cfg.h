/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>

//on-chip
#define FLASH_SIZE_GRANULARITY_128K   (128 * 1024)
#define FLASH_SIZE_GRANULARITY_768K   (768 * 1024)

//extra-chip
#define NOR_FLASH_DEV_NAME             FAL_USING_NOR_FLASH_DEV_NAME
extern struct fal_flash_dev 		   		 nor_flash0;


extern const struct fal_flash_dev stm32f4_onchip_flash;

#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
	&stm32f4_onchip_flash,                             \
	&nor_flash0,                                    \
}

/* ====================== Partition Configuration ========================== 
* bootloader  128K
* app  		768K
* ota  		768K
*************************************************************************/

#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */

#define FAL_PART_TABLE                                                                                                     \
{                                                                                                                          \
    {FAL_PART_MAGIC_WROD, "bootloader", "onchip_flash",  0 , FLASH_SIZE_GRANULARITY_128K , 0}, \
    {FAL_PART_MAGIC_WROD, "app",        "onchip_flash",  128 * 1024 , FLASH_SIZE_GRANULARITY_768K , 0}, \
		{FAL_PART_MAGIC_WROD, "ota", NOR_FLASH_DEV_NAME,  0 , FLASH_SIZE_GRANULARITY_768K , 0}, \
    {FAL_PART_MAGIC_WROD, "factory",        NOR_FLASH_DEV_NAME,  768 * 1024 , FLASH_SIZE_GRANULARITY_768K, 0}, \
		{FAL_PART_MAGIC_WROD, "filesystem",   NOR_FLASH_DEV_NAME, (768 * 1024) * 2 , 31232 * 1024, 0}, \
}

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
