#include <rtthread.h>
#include <rtdevice.h>

#include "lvgl.h"
#include "ui_utils.h"

#ifdef RT_USING_ENCODER_INPUTDEV

#include "mod_input.h"
#include "lv_port_indev.h"
#include <board.h>

#define ENC_IRQ_PIN    GET_PIN(C, 4)

void HW_EncoderReadCount(M_InputOpr *m_Encoder)
{
	M_EncoderPara *tPara = (M_EncoderPara *)m_Encoder->user_data;

	/* 读取编码器旋转方向 */
	rt_device_control(m_Encoder->InputDev, PULSE_ENCODER_CMD_GET_DIRECTTION, (void *)&tPara->direction);
	//rt_kprintf("get direction %d\n",tPara->direction);
	rt_sem_release(&m_Encoder->IrqSem);
}

static rt_err_t Encoder_Rx_IrqHandler(rt_device_t dev, rt_size_t size)
{
	M_InputOpr *m_Encoder = (M_InputOpr *)dev->user_data;
	m_Encoder->event_proc(m_Encoder);
	return RT_EOK;
}


static void enc_Press_callback(void *args)
{
	/* 读取编码器旋转方向 */
#if USING_ENC_IRQ_NORMAL	
	M_InputOpr *m_Encoder = (M_InputOpr *)args;
	M_EncoderPara *tPara = (M_EncoderPara *)m_Encoder->user_data;
	tPara->direction = ENCODER_BTN_ENTER;
	rt_sem_release(&m_Encoder->IrqSem);
#else
	//弹出对话框或直接发送
	if(ui_Dialog.cont != NULL && _settingUI.bIsUIon)
	{
		if(_settingUI.iSwitchs & (1 << BIT_AUTO_LOCK))
			lv_label_set_text(ui_Dialog.title, "是否解除锁定?");
		else
			lv_label_set_text(ui_Dialog.title, "是否开启自动锁?");
		Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_LockOpenLaser);
	}	
#endif
	return;
}



int HW_EncoderInputRegister(M_listObject *list, void *obj)
{
	int ret;
	lv_indev_drv_t *p_indevDrv = (lv_indev_drv_t *)obj;
	
	M_InputOpr *m_Encoder = (M_InputOpr *)rt_malloc(sizeof(M_InputOpr));
	if(RT_NULL == m_Encoder)
	{
	    rt_kprintf("m_Encoder malloc Fail\n");
		return -1;
	}
	/********************************************
	*0. 查找并打开(只读方式)脉冲编码器设备 
	*********************************************/
	m_Encoder->InputDev = rt_device_find(PULSE_ENCODER_DEV_NAME);
	if(m_Encoder->InputDev == RT_NULL)
	{
		rt_kprintf("encoder Register failed! can't find %s device!\n", PULSE_ENCODER_DEV_NAME);
		rt_free(m_Encoder);
		return -1;
	}
	//添加私有数据
	m_Encoder->InputDev->user_data = (M_InputOpr *)m_Encoder;
	ret = rt_device_open(m_Encoder->InputDev, RT_DEVICE_OFLAG_RDONLY);
	if (ret != RT_EOK)
	{
		rt_kprintf("open %s device failed!\n", PULSE_ENCODER_DEV_NAME);
		rt_free(m_Encoder);
		return -1;
	}
    ret = rt_sem_init(&m_Encoder->IrqSem, "encSem", 0, RT_IPC_FLAG_FIFO);
    if (ret != RT_EOK)
    {
        rt_kprintf("init encoder_Sem failed!\n");
		rt_free(m_Encoder);
		return -1;
    }
	/*****************************************
	*1. private数据赋值, 不需要外部传参
	******************************************/ 	
	m_Encoder->user_data = (M_EncoderPara *)rt_calloc(1, sizeof(M_EncoderPara));
	if(m_Encoder->user_data == NULL)
	{
		rt_free(m_Encoder);
		return -1;
	}	
	/*********************
	*2. public数据赋值
	*********************/	
	m_Encoder->id = ENCODER_DEV_ID;
	m_Encoder->event_proc = HW_EncoderReadCount;
	/*********************
	*3. lvgl 输入驱动private数据赋值
	*********************/		
	p_indevDrv->user_data = (M_InputOpr *)m_Encoder;

	/*挂接脉冲接收中断函数*/
    rt_device_set_rx_indicate(m_Encoder->InputDev, Encoder_Rx_IrqHandler);
	/*****************************************
	*  初始化中断脚 
	*******************************************/
	rt_pin_mode(ENC_IRQ_PIN, PIN_MODE_INPUT_PULLUP);	
	rt_pin_attach_irq(ENC_IRQ_PIN, PIN_IRQ_MODE_FALLING, enc_Press_callback, (void *)m_Encoder);
	rt_pin_irq_enable(ENC_IRQ_PIN, PIN_IRQ_ENABLE);
	
	M_listNode *node = list_rpush(list, list_node_new((void *)m_Encoder));
	if(node == NULL)
		return -1;
	rt_kprintf("open %s device success!\n", PULSE_ENCODER_DEV_NAME);
	return 0;
}

#endif

