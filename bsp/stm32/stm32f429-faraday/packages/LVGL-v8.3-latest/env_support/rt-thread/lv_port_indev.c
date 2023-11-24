/*********************
 *      INCLUDES
 *********************/
#include <board.h>
#include "lv_port_indev.h"
#include "lvgl.h"
#include "touch.h"

#include "application.h"
#include "ui_utils.h"



#if defined(RT_USING_TS_INPUTDEV)

lv_indev_t* ts_indev_obj;
static lv_indev_drv_t ts_indev_drv;
static struct rt_touch_data read_data[1];

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
	M_InputOpr* pDev = (M_InputOpr* )indev_drv->user_data;

    int ret = rt_device_read(pDev->InputDev, 0, read_data, 1);
	if(!ret || read_data->event == RT_TOUCH_EVENT_NONE)
		return;
    data->point.x = read_data->x_coordinate;
    data->point.y = read_data->y_coordinate;
    if (read_data->event == RT_TOUCH_EVENT_UP)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
       data->state = LV_INDEV_STATE_PR;
    }    
}

static void touchpad_init(lv_indev_drv_t *indev_drv)
{
	MainCore.new_object(&g_tLaserManager, CLASS_INPUT, DEV_TYPE_TS, (void *)indev_drv);
}
#endif


#if defined(RT_USING_ENCODER_INPUTDEV)

lv_indev_t* enc_indev_obj;
static lv_indev_drv_t enc_indev_drv;


static void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
	M_InputOpr* pDev = (M_InputOpr* )indev_drv->user_data;

	if(RT_EOK == rt_sem_trytake(&pDev->IrqSem))
	{		
		M_EncoderPara *para = (M_EncoderPara *)pDev->user_data;
		switch(para->direction) 
		{
#if USING_ENC_IRQ_NORMAL
			case ENCODER_BTN_ENTER:
				data->key = LV_KEY_USER_ENTER;
				data->state = LV_INDEV_STATE_PR;
			break;
#endif
			case ENCODER_TRUN_LEFT:
				data->key = LV_KEY_LEFT;
				data->state = LV_INDEV_STATE_REL;
			    data->enc_diff = -1;
			break;
			case ENCODER_TRUN_RIGHT:
				data->key = LV_KEY_RIGHT;
				data->state = LV_INDEV_STATE_REL;
				data->enc_diff = 1;		
			break;
		}
	}
	else{
		data->enc_diff = 0;
		data->state = LV_INDEV_STATE_REL;
	}
}

static void encoder_init(lv_indev_drv_t *indev_drv)
{
	MainCore.new_object(&g_tLaserManager, CLASS_INPUT, DEV_TYPE_ENCODER, (void *)indev_drv);
}
#endif

#if defined(RT_USING_KEYPAD_INPUTDEV)

lv_indev_t* key_indev_obj;
static lv_indev_drv_t key_indev_drv;

static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
	static uint8_t last_key = 0xff;
	uint8_t act_key = 0xff;
	M_InputOpr* pDev = (M_InputOpr* )indev_drv->user_data;
	M_KeyPadPara *para = (M_KeyPadPara *)pDev->user_data;
	
	if(para->last_btn != 0xff)
	{
		data->state = LV_INDEV_STATE_PR;
		switch(para->last_btn) 
		{
			case 0xfd:
				act_key = LV_KEY_UP;
			break;
			case 0xfb:
				act_key = LV_KEY_DOWN;
			break;
			case 0xfe:
				act_key = LV_KEY_LEFT;
			break;
			case 0xf7:
				act_key = LV_KEY_RIGHT;
			break;
			case 0xdf:
				act_key = LV_KEY_ESC;
			break;
			//组合键
			case 0xdd:
				act_key = LV_KEY_USER_LOGIN;
				if(login_win.loginBtn != NULL)
					lv_event_send(login_win.loginBtn, LV_EVENT_VALUE_CHANGED, NULL);
			break;
			case 0xef:
				data->state = LV_INDEV_STATE_REL;
			break;
			
			default:
				data->state = LV_INDEV_STATE_REL;
			break;
		}
		last_key = act_key;
	} else {
		data->state = LV_INDEV_STATE_REL;
	}
  	data->key = last_key;
}

static void keypad_init(lv_indev_drv_t *indev_drv)
{
	MainCore.new_object(&g_tLaserManager, CLASS_INPUT, DEV_TYPE_KEYPAD, (void *)indev_drv);
}
#endif

/***********************************************************************************/
void lv_port_indev_init(void)
{
#if defined(RT_USING_TS_INPUTDEV)
    lv_indev_drv_init(&ts_indev_drv);
	touchpad_init(&ts_indev_drv);
    ts_indev_drv.type = LV_INDEV_TYPE_POINTER;
    ts_indev_drv.read_cb = touchpad_read;
    ts_indev_obj = lv_indev_drv_register(&ts_indev_drv);
#endif

#if defined(RT_USING_ENCODER_INPUTDEV)
    lv_indev_drv_init(&enc_indev_drv);
    encoder_init(&enc_indev_drv);
    enc_indev_drv.type = LV_INDEV_TYPE_ENCODER;
    enc_indev_drv.read_cb = encoder_read;
    enc_indev_obj = lv_indev_drv_register(&enc_indev_drv);	
#endif

#if defined(RT_USING_KEYPAD_INPUTDEV)
	lv_indev_drv_init(&key_indev_drv);
    keypad_init(&key_indev_drv);
    key_indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    key_indev_drv.read_cb = keypad_read;
    key_indev_obj = lv_indev_drv_register(&key_indev_drv);
#endif
}








