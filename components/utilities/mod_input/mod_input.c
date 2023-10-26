#include "mod_input.h"


M_listNode *Input_DevFind(M_listObject *ptList, uint8_t id)
{
	M_InputOpr *pDev = NULL;
	M_listNode *p_node = NULL;
	
	list_iterator_t *it = list_iterator_new(ptList, LIST_HEAD);
	while ((p_node = list_iterator_next(it))) {
		pDev = (M_InputOpr *)p_node->data;
		if(id == pDev->id) 
			break;
	}
	rt_free(it);
	return p_node;
}


int InputDevInit(M_listObject *InputDev_list, uint8_t id, void *obj)
{	
	int ret = 0;
	
	if(InputDev_list == NULL)
	{
		InputDev_list = list_new();
		if(InputDev_list == NULL)
			return -1;
	}
	switch(id)
	{
#if defined(RT_USING_ENCODER_INPUTDEV)
		case DEV_TYPE_ENCODER:
			/*************************************************
			* 注册编码器设备
			*************************************************/
			ret = HW_EncoderInputRegister(InputDev_list, obj);
			if(ret < 0)
				rt_kprintf("Register Encoder fail!\n");
		break;
#endif
#if defined(RT_USING_TS_INPUTDEV)
		case DEV_TYPE_TS:
			/*************************************************
			* 注册触摸屏设备
			*************************************************/
			ret = HW_TsInputRegister(InputDev_list, obj);
			if(ret < 0)
				rt_kprintf("Register touchPad fail!\n");
		break;
#endif
#if defined(RT_USING_KEYPAD_INPUTDEV)
		case DEV_TYPE_KEYPAD:
			/*************************************************
			* 注册按键设备
			*************************************************/
			ret = HW_KeyInputRegister(InputDev_list, obj);
			if(ret < 0)
				rt_kprintf("Register keyPad fail!\n");
		break;
#endif
		default:
			ret = -1;
		break;
	}
	return ret;
}


