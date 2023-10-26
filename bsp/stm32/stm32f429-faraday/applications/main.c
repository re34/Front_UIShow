/*
 * Copyright (c) 2020-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-3-06     liuxianyou   first version
 */
 
/* RAM = RW  +  ZI  = 14K; (192K RAM)
   ROM = code  +  RW  +  RO = 156K (1M flash)
*/


#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "application.h"
//#include "mod_para.h"
#include "mod_trans.h"
#include "spi_ad7606.h"


extern int lvgl_thread_init(void);

/* RT-Thread 中，系统节拍的长度可以根据 RT_TICK_PER_SECOND 的定义来调整，等于 1/RT_TICK_PER_SECOND 秒 */

/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(B, 1)
MOD_INSERT g_tLaserManager;


void Core_CreateMod(MOD_INSERT* core, int classType, uint8_t id, void *argv)
{
	int ret = 0;
	switch(classType)
	{
		case CLASS_CHIPDAC:
#if 0
			core->mod_dac = DacChipInit(argv);  //mod_dac就是DAC设备的链表头
			if(core->mod_dac == NULL)
			{
				rt_kprintf("Create DAC device fail!\r\n");
			}
#endif
			break;
		case CLASS_INPUT:
			ret = InputDevInit(core->mod_InputList, id, argv);
			if(ret < 0)
			{
				rt_kprintf("Create Input device fail!\r\n");
			}
			break;		
		default:
			break;
	}
}

/*************check ok**********************/
void Core_DeleteMod(MOD_INSERT* core, int mode, char *pcName)
{
    switch(mode)
    {
        case CLASS_CHIPDAC:
            //core->mod_dac = DacChipExit(&core->mod_dac, pcName); //删除节点后，更新链表
            break;
        default:
            break;
    } 
}

T_LaserCore MainCore = {
	Core_CreateMod, 
	Core_DeleteMod, 
};


int main(void)
{
	rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
	rt_kprintf("Version: 4.0.9\r\n");

	trans_modbusInit();
	lvgl_thread_init();
	while(1)
	{
		rt_pin_write(LED0_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED0_PIN, PIN_LOW);
		rt_thread_mdelay(500);
	}
}
