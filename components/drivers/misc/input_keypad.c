#include <rtthread.h>
#include <rtdevice.h>


#ifdef RT_USING_KEYPAD_INPUTDEV

#include "mod_input.h"
#include "lv_port_indev.h"
#include "touch.h"
#include "pcf8574.h"


static rt_err_t Key_Irq_callback(rt_device_t dev, rt_size_t size)
{
	rt_touch_t touch = (rt_touch_t)dev;
	M_InputOpr *p_TouchPad = (M_InputOpr *)dev->user_data;
	//无论按下还是松开都会发送下降沿
	if(PIN_LOW == rt_pin_read(touch->config.irq_pin.pin))
	{
	    rt_sem_release(&p_TouchPad->IrqSem);
	}
    return 0;
}


void key_Irq_monitor(void *parameter)
{
	M_InputOpr* pKeyDev = (M_InputOpr *)parameter;
	M_KeyPadPara *para = (M_KeyPadPara *)pKeyDev->user_data;
	
	while(1)
	{
		rt_sem_take(&pKeyDev->IrqSem, RT_WAITING_FOREVER);
		para->last_btn = rt_device_read(pKeyDev->InputDev, 0, RT_NULL, READ_ALL_BIT);
		if(0xff != para->last_btn)
		{
			para->bIsPress = RT_TRUE;
		}else{
			para->bIsPress = RT_FALSE;
		}
		//rt_kprintf("para->last_btn = 0x%02x\n", para->last_btn);
	}
}


void HW_KeyReadCount(M_InputOpr *m_InputDev)
{
	return;
}


int HW_KeyInputRegister(M_listObject *list, void *obj)
{
	int ret;
	rt_thread_t  key_thread = RT_NULL;	
	lv_indev_drv_t *p_indevDrv = (lv_indev_drv_t *)obj;

	M_InputOpr *p_KeyPad = (M_InputOpr *)rt_malloc(sizeof(M_InputOpr));
	if(RT_NULL == p_KeyPad)
	{
	    rt_kprintf("keypad malloc Fail\n");
		return -1;
	}
	/********************************************
	*0. 查找并打开(只读方式)脉冲编码器设备 
	*********************************************/
	p_KeyPad->InputDev = rt_device_find(KEY_DEV_NAME);
	if(p_KeyPad->InputDev == RT_NULL)
	{
		rt_kprintf("touch Register failed! can't find %s device!\n", KEY_DEV_NAME);
		return -1;
	}
	/*****************************************
	*1. private数据赋值
	******************************************/ 	
	p_KeyPad->user_data = (M_KeyPadPara *)rt_calloc(1, sizeof(M_KeyPadPara));
	M_KeyPadPara *para = (M_KeyPadPara *)p_KeyPad->user_data;
	para->last_btn = 0xff;
	/*********************
	*2. public数据赋值
	*********************/	
	p_KeyPad->id = KEY_DEV_ID;
	p_KeyPad->event_proc = HW_KeyReadCount;
	/*****************************************
	*3. device private数据赋值
	******************************************/ 
	p_KeyPad->InputDev->user_data = (M_InputOpr *)p_KeyPad;
	/*****************************************
	*4. 创建信号量及监听线程
	******************************************/
	ret = rt_sem_init(&p_KeyPad->IrqSem, "keySem", 0, RT_IPC_FLAG_FIFO);
	if (ret != RT_EOK)
	{
		rt_kprintf("init tsSem failed!\n");
		return -1;
	}
	key_thread = rt_thread_create("key_thread", key_Irq_monitor, (void *)p_KeyPad, 512, 25, 5);
	if (key_thread != RT_NULL)
		rt_thread_startup(key_thread);
	/*********************
	*6. lvgl 输入驱动private数据赋值
	*********************/		
	p_indevDrv->user_data = (M_InputOpr *)p_KeyPad;
	/*****************************************
	*7. 挂接目标接收中断函数
	******************************************/	
	rt_device_set_rx_indicate(p_KeyPad->InputDev, Key_Irq_callback);	
	if(rt_device_open(p_KeyPad->InputDev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
	{
		rt_kprintf("open device failed!");
		return -1;
	}	
	M_listNode *node = list_rpush(list, list_node_new((void *)p_KeyPad));
	if(node == NULL)
		return -1;
	return 0;
}

#endif

