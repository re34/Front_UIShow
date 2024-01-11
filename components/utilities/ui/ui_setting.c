/****************************************************************
*  更新数据逻辑：
*
*	spibox类标签 (支持触摸屏幕+/-、 上下按键、旋钮)：
*  		1. 屏幕进入配置界面时，_initVal从 ui_cfgVal[]获取配置值，并刷新界面;
*  		2. 发送给设备数据，界面数据更新，不更新ui_cfgVal[]和 _initVal
*  		3. 锁定状态切换后，执行一次更新，modbus数据更新给ui_cfgVal[], ui_cfgVal[]更新界面，不更新_initVal

*
*   arc_bar类按钮(支持触摸屏幕、独立按键)：：
*		1. 屏幕进入配置界面时，_initVal从 ui_cfgVal[]获取配置值(锁定按钮还需配合ui_sample[])，并刷新界面;
*		2. 发送给设备开关数据(0/1)，界面更新，更新ui_cfgVal[]和 _initVal

*		3. 锁定状态切换后，更新_initVal
***************************************************************************************************/



#include <stdlib.h>
#include "lvgl.h"
#include "lv_port_indev.h"
#include "ui_utils.h"
#include "mod_trans.h"



const char* label_list[PARAM_ITEM_NUMS_END] = {
    "电流设置(mA)",
	"温度设置(℃)",
	"扫描频率(HZ)",
	"扫描幅值(V)",
	"扫描偏置(V)",
	"调制相位",
	"调制频率(HZ)",
/******************************/
	"电流最大值(mA)",
	"温度最大值(℃)",	
	"扫描电流(mA)",
	"自动锁峰尖识别值",
	"电流工作点(mA)",
	"温度工作点(℃)",		
	"调制频率工作点(HZ)",	
	"调制相位工作点",
};

const char* Switch_list[RELAY_NUMS] = {
	"开关1",
	"112",
	"113",
	"开关4",
	"115",
	"116",
	"开关7",
};

const char *btn_names[2] = {
	MY_ICON_MANUAL_UNLOCK,
	MY_ICON_AUTO_PEAK,
};

const char* view_icons[3] = {
	MY_ICON_CURRENT,
	MY_ICON_TEMPER,
	MY_ICON_DDS,
};

struct _ui_Setting _settingUI;
enc_flewChk_t enc_Chker;



static void spinbox_Send_updateVal(uint8_t index, int32_t value)
{
	ui_cfgVal[1 + index].recvDate = value;		
	Gui_SendMessge(uart_mq, ITEM_ADDR_HEAD + 2 * index, 2, E_Modbus_Write, value);
}




void spinbox_judge_val(uint8_t index, bool enable)
{
	int32_t limitVal = 0;
	int ret = 0;
	int32_t nowVal = lv_spinbox_get_value(_settingUI._mods[index]->_mObj);
	switch(index)
	{
		case Item_Current:
		case Item_Temper:
			limitVal = lv_spinbox_get_value(_settingUI._mods[index + ALARM_OFFSET]->_mObj);
			if(nowVal > limitVal)
			{
				ret = -1;
			}	
		break;
		case Item_PztBias:
			limitVal = lv_spinbox_get_value(_settingUI._mods[index - 1]->_mObj);
			if((nowVal + limitVal) > PZT_AMP_MAX)
			{
				ret = -1;						
			}
		break;
		default:				
		break;
	}
	if(!ret)
		spinbox_Send_updateVal(index, nowVal);
	else
	{
		lv_spinbox_decrement(_settingUI._mods[index]->_mObj);
		//弹窗(加号, 减法不需要)
		if(ui_Dialog.cont != NULL && enable)
		{
			lv_label_set_text(ui_Dialog.title, "输入错误!");
			Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_Tips);
		}
	}
}



void sw_flush_val(lv_obj_t *obj, uint8_t index, uint32_t val)
{
	if((1 << index) & val)
		lv_obj_add_state(obj, LV_STATE_CHECKED);
	else
		lv_obj_clear_state(obj, LV_STATE_CHECKED);			
}


void spinbox_overFlow_send(bool enable)
{
	enc_Chker.idleCnt = 0;
	enc_Chker.bIsDataSend = false;
	enc_Chker.bIsIncEnabled = enable;
	if(enc_Chker.bIsTimerRun == false)
	{
		enc_Chker.bIsTimerRun = true;
		//开启空闲超时检测
		lv_timer_resume(enc_Chker.enc_flewTimer);
	}
}


/*****************************************************************************************/
static void lv_spinbox_inc_event_cb(lv_event_t * e)
{		
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
        lv_spinbox_increment(_settingUI._mods[index]->_mObj);
		spinbox_judge_val(index, true);
    }
}

static void lv_spinbox_dec_event_cb(lv_event_t * e)
{

    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
		lv_spinbox_decrement(_settingUI._mods[index]->_mObj);
		spinbox_judge_val(index, false);
    }
}

static void spinbox_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_spinbox_t * s_spinbox = (lv_spinbox_t *)obj;

	struct m_attr_t *p_attr = (struct m_attr_t *)obj->user_data;
	//if (LV_INDEV_TYPE_ENCODER == lv_indev_get_type(lv_indev_get_act()))
    if (code == LV_EVENT_CLICKED)
    {
		if(_settingUI.bIsAdmin == Authority_User)
		{
			uint8_t id = lv_tabview_get_tab_act(_settingUI._tabCont);
			if(id > 1){
				lv_label_set_text(ui_Dialog.title, "请先登录!");
				Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_Tips_Move);
				return;
			}
		}
		lv_group_focus_obj(obj);
		lv_obj_add_state(obj, LV_STATE_FOCUSED);
		lv_group_set_editing(lv_group_get_default(), true);	
		//设置要发送的目标序号
		enc_Chker.targetInx = p_attr->itemIndex;
    }	
	else if(code == LV_EVENT_KEY)
	{
		uint32_t key = lv_event_get_key(e);
		if (LV_KEY_USER_OK == key)
		{
			lv_indev_wait_release(lv_indev_get_act());
			lv_obj_clear_state(obj, LV_STATE_FOCUSED);
			lv_indev_reset(NULL, obj);
			lv_group_set_editing(lv_group_get_default(), false);
			/***************************************
			*  1. 数据校验是否合理, 发送串口数据
			**************************************/
			//更新数据
			spinbox_Send_updateVal(p_attr->itemIndex, s_spinbox->value);
			if(enc_Chker.bIsTimerRun == true)
			{
				lv_timer_pause(enc_Chker.enc_flewTimer);
				enc_Chker.bIsTimerRun = false;
			}
		}		
	}
}
/*****************************************************************************************/

void spinBox_style_init(tab_module_t* t_objBox)
{
	spinContent_style_init(UI_Type_LD, t_objBox, &label_list[0], spinbox_event_cb);
	
	lv_obj_t * btn = spinBtn_style_init(t_objBox, lv_spinbox_inc_event_cb);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
	btn = spinBtn_style_init(t_objBox, lv_spinbox_dec_event_cb);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_LEFT_MID, -8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
}


/*****************************************************************************************/

static void sw_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
	{
        uint8_t val = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
		uint8_t index = *((uint8_t *)sw->user_data);
		if(val)
		{			
			_settingUI.iSwitchs = (_settingUI.iSwitchs & 0x7FF)|(1 << index);
		}else{
			_settingUI.iSwitchs = (_settingUI.iSwitchs & 0x7FF)&(~(1 << index));		
		}
		Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, 2, E_Modbus_Write, _settingUI.iSwitchs);	
    }
}


void sw_style_init(tab_module_t* t_objBox)
{
	struct m_attr_t *p_attr = &t_objBox->_attr;

	lv_obj_remove_style_all(t_objBox->_root);
	lv_obj_set_size(t_objBox->_root, 90, 60);

	lv_obj_t *lbl_status = lv_label_create(t_objBox->_root);
	lv_obj_set_style_text_color(lbl_status, lv_color_white(), LV_PART_MAIN);
	lv_label_set_text(lbl_status, Switch_list[p_attr->itemIndex]);
	lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 0);

	lv_obj_t *sw = lv_switch_create(t_objBox->_root);
	lv_obj_set_size(sw, 45, 22);
	lv_obj_align_to(sw, lbl_status, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
	sw->user_data = (void *)&p_attr->itemIndex;
	lv_obj_add_event_cb(sw, sw_event_cb, LV_EVENT_CLICKED, NULL);
	if(t_objBox->_attr._initVal)
		lv_obj_add_state(sw, LV_STATE_CHECKED);
	else
		lv_obj_clear_state(sw, LV_STATE_CHECKED);
    lv_obj_set_style_border_width(sw, 2, LV_PART_KNOB);
    lv_obj_set_style_border_color(sw, lv_color_hex(0x29a6ac), LV_PART_KNOB);
	lv_group_remove_obj(sw);
	t_objBox->_mObj = sw;
}
/*****************************************************************************************/

tab_module_t *subTab_create(lv_obj_t * parent, uint8_t index, uint8_t tabType, void *ext_data)
{
    tab_module_t* t_tabBox = (tab_module_t *)rt_malloc(sizeof(tab_module_t));
    if (t_tabBox != NULL)
    {
		t_tabBox->_attr.itemIndex = index;
		t_tabBox->_attr.range_min = 0;			
		//频率设置项
		if(index == 2 || index == 6 || index == 13)
		{
			t_tabBox->_attr.bHasDot = false;
			t_tabBox->_attr.range_min = 1;
			t_tabBox->_attr.range_max = 6000000;
		//PZT扫描偏置
		}else if(index == 4)
		{
			t_tabBox->_attr.bHasDot = true;
			t_tabBox->_attr.range_max = 14000;
		}
		//pzt扫描幅值		
		else if(index == 3)
		{
			t_tabBox->_attr.bHasDot = true;
			t_tabBox->_attr.range_max = 7000;
		}
		//dds相位
		else if(index == 5 || index == 14)
		{
			t_tabBox->_attr.bHasDot = false;
			t_tabBox->_attr.range_max = 360;
		}else{
			t_tabBox->_attr.bHasDot = true;
			t_tabBox->_attr.range_max = 999999;
		}			
        t_tabBox->_root = lv_obj_create(parent);
		if(tabType == ENUM_TYPE_TXT)
		{
			//ui_cfgVal[0]为所有开关状态
			t_tabBox->_attr._initVal = ui_cfgVal[1 + index].recvDate;
			t_tabBox->_mObj = lv_spinbox_create(t_tabBox->_root);
			spinBox_style_init(t_tabBox);
		}else{
			t_tabBox->_attr._initVal = (1 << index) & _settingUI.iSwitchs;
			sw_style_init(t_tabBox);
		}
    }
    return t_tabBox;
}



/*****************************************************************************************/


static void anim_arc_process(void* var, int32_t value)
{
	lv_arc_set_end_angle((lv_obj_t *)var, value); //设置弧形控件的结束角度
}


static void anim_arc_finish(lv_anim_t *a)
{
	lv_obj_t *label_cont = lv_obj_get_child((lv_obj_t *)a->var, 0);
	lv_obj_t *label = lv_obj_get_child(label_cont, 0);
	struct m_attr_t *p_attr = (struct m_attr_t *)label_cont->user_data;
	//自动锁需要变换图片及颜色， 开关机只需要变颜色
	if(p_attr->itemIndex)
	{
		switch(p_attr->_initVal)
		{
			case Lock_Proc_Off:
				//关机/失锁(红色)
				lv_label_set_text(label, MY_ICON_UNLOCK);
				lv_anim_set_values(&_settingUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_min, p_attr->range_max / 2);
				lv_obj_set_style_text_color(label, lv_color_hex(0xd74047), LV_PART_MAIN);
			break;
			case Lock_Proc_handling: //确定后半程
				lv_label_set_text(label, MY_ICON_UNLOCK);
				lv_anim_set_values(&_settingUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_max / 2, p_attr->range_max);
				lv_obj_set_style_text_color(label, lv_color_hex(0xd74047), LV_PART_MAIN);
			break;
			case Lock_Proc_On: 
				lv_label_set_text(label, MY_ICON_LOCK);
				//下一次动画方向
				lv_anim_set_values(&_settingUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_max, p_attr->range_min);			
				lv_obj_set_style_text_color(label, lv_color_hex(0x4ab3b0), LV_PART_MAIN); //锁定（绿色）
			break;

		}
	}else{
		//开关机选项
		if(p_attr->_initVal)
		{
			lv_anim_set_values(&_settingUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_max, p_attr->range_min);
			//开机(绿色)
			lv_obj_set_style_text_color(label, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
		}
		else{
			lv_anim_set_values(&_settingUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_min, p_attr->range_max);
			//关机(红色)
			lv_obj_set_style_text_color(label, lv_color_hex(0xd74047), LV_PART_MAIN);			
		}
	}
}




static void switchLock_event_cb(lv_event_t *e)
{
	lv_obj_t *label_cont = lv_event_get_target(e);
	struct m_attr_t *p_attr = (struct m_attr_t *)label_cont->user_data; 

	if(e->code == LV_EVENT_CLICKED)
	{
		if(ui_Dialog.cont != NULL)
		{
			if(p_attr->itemIndex)
			{
				if(_settingUI.iSwitchs & (1 << BIT_AUTO_LOCK))
					lv_label_set_text(ui_Dialog.title, "是否解除锁定?");
				else
					lv_label_set_text(ui_Dialog.title, "是否开启自动锁?");
				ui_Dialog.bCheckFlag = Dialog_Type_LockAuto;
			}else{
				if(_settingUI.iSwitchs & (1 << BIT_PowerSw))
					lv_label_set_text(ui_Dialog.title, "是否关闭电源?");
				else
					lv_label_set_text(ui_Dialog.title, "是否打开电源?");
				ui_Dialog.bCheckFlag = Dialog_Type_Power;
			}
			Gui_DialogShow(&ui_Dialog, NULL, ui_Dialog.bCheckFlag);
		}	
	}
	else if(e->code == LV_EVENT_VALUE_CHANGED) //锁定事件专用
	{	
		if(p_attr->itemIndex == Type_Power)//开关机键功能
		{
			if(ui_Dialog.bCheckFlag == Dialog_Type_LockOpenLaser)
			{		
				//存在已开机 未锁定的情况
				if(p_attr->_initVal ^ _settingUI._mods_sp[Type_Lock]->_attr._initVal)
				{
					if(_settingUI._mods_sp[Type_Lock]->_attr._initVal == Lock_Proc_Off)
						return;
				}
			}
			if(p_attr->_initVal == Power_Proc_On)
			{
				p_attr->_initVal = Power_Proc_Off;
				_settingUI.iSwitchs = (_settingUI.iSwitchs & 0x7FF)&(~(1 << BIT_PowerSw));	
			}else{
				p_attr->_initVal = Power_Proc_On;
				_settingUI.iSwitchs = (_settingUI.iSwitchs & 0x7FF)|(1 << BIT_PowerSw); 
			}
			Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, 2, E_Modbus_Write, _settingUI.iSwitchs);	
			lv_anim_start(&_settingUI._mods_sp[p_attr->itemIndex]->anim);			
		}
		else{
			if(p_attr->_initVal != Lock_Proc_Off || ui_Dialog.bCheckFlag == Dialog_Type_LockManual)
			{	
				uint8_t pre_val = p_attr->_initVal;
				/************************
				*2. 主动失锁，更新状态
				************************/			
				p_attr->_initVal = Lock_Proc_Off;
				_settingUI.iSwitchs = (_settingUI.iSwitchs & 0x7FF)&(~(1 << BIT_AUTO_LOCK));	
				ui.bottomInfo.bFirstEntryLock = false;
				ui_sample[LOCK_STATE_ADDR].recvDate = 0; 
				_settingUI.iSwitchs |= (1 << BIT_SCAN);
				Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, 2, E_Modbus_Write, _settingUI.iSwitchs |(1 << BIT_UNLOCK));					
				/************************
				*3. 关闭4个继电器
				************************/					
				lv_obj_clear_state(_settingUI._mods_sw[1]->_mObj, LV_STATE_CHECKED);
				lv_obj_clear_state(_settingUI._mods_sw[2]->_mObj, LV_STATE_CHECKED);		
				lv_obj_clear_state(_settingUI._mods_sw[4]->_mObj, LV_STATE_CHECKED);
				lv_obj_clear_state(_settingUI._mods_sw[5]->_mObj, LV_STATE_CHECKED);
				_settingUI.iSwitchs &= ~0x36;	
				/************************
				*4. 打开扫描
				************************/
				lv_obj_set_style_bg_color(_settingUI._ScanObj, lv_color_hex(0x3b67b0), LV_PART_MAIN);
				/************************
				*5. 调整动画值
				************************/
				if(ui_Dialog.bCheckFlag >= Dialog_Type_LockAuto)
				{
					if(pre_val == Lock_Proc_handling)
						lv_anim_set_values(&_settingUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_max / 2, p_attr->range_min);
					lv_anim_start(&_settingUI._mods_sp[p_attr->itemIndex]->anim);
				}
				return;	
			}
			else{
				_settingUI.iSwitchs = (_settingUI.iSwitchs & 0x7FF)|(1 << BIT_AUTO_LOCK);
				/************************
				*1. 关闭扫描
				************************/				
				p_attr->_initVal = Lock_Proc_handling;
				_settingUI.iSwitchs &= ~(1 << BIT_SCAN);
				lv_obj_set_style_bg_color(_settingUI._ScanObj, lv_color_hex(0xd74047), LV_PART_MAIN);		
			}
			Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, 2, E_Modbus_Write, _settingUI.iSwitchs);	
			lv_anim_start(&_settingUI._mods_sp[p_attr->itemIndex]->anim);	
		}
	}
}


sw_module_t *switchLock_create(lv_obj_t * parent, uint8_t index, void *ext_data)
{
    sw_module_t* t_swMod = (sw_module_t *)rt_malloc(sizeof(sw_module_t));
	if(t_swMod)
	{
		struct m_attr_t *p_attr = &t_swMod->_attr;
		
		p_attr->range_min = RANGE_MIN;
		p_attr->range_max = RANGE_MAX;
		p_attr->itemIndex = index;		
		lv_obj_t * arc_obj = lv_arc_create(parent);
		lv_obj_set_size(arc_obj, 85, 85);
		lv_arc_set_bg_angles(arc_obj, p_attr->range_min, p_attr->range_max);
		lv_arc_set_rotation(arc_obj, 90);
		lv_obj_set_style_arc_color(arc_obj, lv_color_hex(0x53597d), LV_PART_MAIN);
		lv_obj_remove_style(arc_obj, NULL, LV_PART_KNOB);		      				//移除按钮
		lv_obj_set_style_arc_width(arc_obj, 14, LV_PART_INDICATOR);  //设置背景圆弧粗细
		lv_obj_set_style_arc_width(arc_obj, 14, LV_PART_MAIN);       //设置前景圆弧粗细
		lv_obj_clear_flag(arc_obj, LV_OBJ_FLAG_CLICKABLE);		  	  //禁止按键
		
		lv_obj_t * arc_label_cont = lv_obj_create(arc_obj);
		lv_obj_set_size(arc_label_cont, 55, 55);
		lv_obj_set_style_bg_color(arc_label_cont, lv_color_hex(0x3b4066), LV_PART_MAIN);
		lv_obj_set_style_border_width(arc_label_cont, 0, LV_PART_MAIN);
		lv_obj_set_style_radius(arc_label_cont, 30, LV_PART_MAIN);
		lv_obj_align(arc_label_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_flag(arc_label_cont, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_clear_flag(arc_label_cont, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_event_cb(arc_label_cont, switchLock_event_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_add_event_cb(arc_label_cont, switchLock_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
		arc_label_cont->user_data = (void *)p_attr;
		t_swMod->_mObj = arc_label_cont;

		lv_obj_t * arc_label = lv_label_create(arc_label_cont);
		lv_obj_set_style_text_font(arc_label, &font_symbol_20, LV_PART_MAIN);
		lv_obj_align(arc_label, LV_ALIGN_CENTER, 0, 0);

		lv_anim_init(&t_swMod->anim);													// 初始化动画
		lv_anim_set_var(&t_swMod->anim, arc_obj);										// 设置动画对象为arc_obj
		lv_anim_set_exec_cb(&t_swMod->anim, (lv_anim_exec_xcb_t)anim_arc_process);		//设置动画回调函数
		lv_anim_set_ready_cb(&t_swMod->anim, anim_arc_finish);		
		//锁定键初始值设定
		if(p_attr->itemIndex)
		{
			//异或为1 说明执行了锁定操作，但没锁上
			uint8_t bitVal = (uint8_t)(((1 << BIT_AUTO_LOCK) & _settingUI.iSwitchs) >> BIT_AUTO_LOCK);
			uint8_t lockstat = (uint8_t)(ui_sample[LOCK_STATE_ADDR].recvDate ^ bitVal);
			if(lockstat)
				p_attr->_initVal = Lock_Proc_handling;
			else{
				p_attr->_initVal = bitVal;
			}
			switch(p_attr->_initVal)
			{
				case Lock_Proc_handling:
					lv_arc_set_angles(arc_obj, p_attr->range_min, p_attr->range_max / 2);
					lv_label_set_text(arc_label, MY_ICON_UNLOCK);					
				break;
				case Lock_Proc_Off:
					lv_arc_set_value(arc_obj, 0);
					lv_label_set_text(arc_label, MY_ICON_UNLOCK);
				break;
				case Lock_Proc_On:
					lv_arc_set_angles(arc_obj, p_attr->range_min, p_attr->range_max);	
					lv_label_set_text(arc_label, MY_ICON_LOCK);
				break;
			}
			lv_obj_align(arc_obj, LV_ALIGN_TOP_RIGHT, -5, 20);	
		}else {
			p_attr->_initVal = (uint8_t)(((1 << BIT_PowerSw) & _settingUI.iSwitchs) >> BIT_PowerSw);
			if(p_attr->_initVal)
				lv_arc_set_angles(arc_obj, p_attr->range_min, p_attr->range_max);
			else
				lv_arc_set_value(arc_obj, 0);
			lv_obj_align(arc_obj, LV_ALIGN_TOP_LEFT, 5, 20);
			lv_label_set_text(arc_label, MY_ICON_POWER);
		}
		if(p_attr->_initVal == Lock_Proc_On){
			//开机/锁定(绿色)
			lv_obj_set_style_text_color(arc_label, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
			lv_anim_set_time(&t_swMod->anim, 3000);								   //设置动画时间3秒
			lv_anim_set_values(&t_swMod->anim, p_attr->range_max, p_attr->range_min);
		}else{
			//关机/失锁(红色)
			lv_obj_set_style_text_color(arc_label, lv_color_hex(0xd74047), LV_PART_MAIN);
			if(p_attr->itemIndex){
				lv_anim_set_time(&t_swMod->anim, 1500);								// 设置动画时间1.5秒
				if(p_attr->_initVal == Lock_Proc_Off)
					lv_anim_set_values(&t_swMod->anim, p_attr->range_min, p_attr->range_max / 2);
				else {
					lv_anim_set_values(&t_swMod->anim, p_attr->range_max / 2, p_attr->range_max);
				}
			}else{
				lv_anim_set_time(&t_swMod->anim, 3000);								// 设置动画时间3秒
				lv_anim_set_values(&t_swMod->anim, p_attr->range_min, p_attr->range_max);
			}
		}		
	}
	return t_swMod;
}

/*****************************************************************************************
*	切换至采样界面
********************************************************************************/
static void btnSample_event_cb(lv_event_t* event)
{
	/*设置主页面为设置所在页*/
	if(event->code == LV_EVENT_CLICKED)
	{
		lv_obj_t *sample_tile = lv_obj_get_parent(ui.topInfo.cont);
		lv_obj_t *root = lv_obj_get_parent(sample_tile);
		lv_obj_set_tile(root, sample_tile, LV_ANIM_ON);
		
		/******************************************
		* 开启采集器刷新定时器, 关闭锁定监控 已在页面切换中实现
		* lv_timer_resume(ui.collect_timer);
		* lv_timer_pause(_settingUI.lock_monitor);
		*****************************************/
		//关闭超时检查
		if(enc_Chker.bIsTimerRun == true)
		{
			lv_timer_pause(enc_Chker.enc_flewTimer);
			enc_Chker.bIsTimerRun = false;
		}
	}
}



/***************************************************************
* 手动失锁操作
*****************************************************************/
static void btnUnlock_event_cb(lv_event_t* e)
{
	//主动失锁操作： 关闭4个继电器，打开扫描，关闭自动锁
	if(e->code == LV_EVENT_CLICKED)
	{
		if(_settingUI.iSwitchs & (1 << BIT_AUTO_LOCK)) //自动锁解锁
		{			
			if(_settingUI._mods_sp[1]->_attr._initVal == Lock_Proc_Off)
			{
				return;
			}
			ui_Dialog.bCheckFlag = Dialog_Type_LockAuto;
		}else{
			ui_Dialog.bCheckFlag = Dialog_Type_LockManual;
		}
		if(ui_Dialog.cont != NULL)
		{
			// 使能 lv_layer_top 点击
			lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
			lv_label_set_text(ui_Dialog.title, "是否解除锁定?");
			lv_obj_clear_flag(ui_Dialog.cont, LV_OBJ_FLAG_HIDDEN);				
            lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);                          // 设置对象透明度
            lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0); // 设置对象颜色
		}			
	}
}

/***************************************************************
* 扫描开关
*****************************************************************/
static void btnPztScan_event_cb(lv_event_t* e)
{
	if(e->code == LV_EVENT_CLICKED)
	{
		if(!(_settingUI.iSwitchs & (1 << BIT_SCAN)))
		{
			//打开扫描
			_settingUI.iSwitchs |= (1 << BIT_SCAN);
			lv_obj_set_style_bg_color(e->target, lv_color_hex(0x3b67b0), LV_PART_MAIN);
		}else{
			//关闭扫描
			_settingUI.iSwitchs &= ~(1 << BIT_SCAN);
			lv_obj_set_style_bg_color(e->target, lv_color_hex(0xd74047), LV_PART_MAIN);
		}
		Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, 2, E_Modbus_Write, _settingUI.iSwitchs);	
	}

}




void Func_ManualBtnInit(lv_obj_t *parent, uint8_t index)
{
	lv_obj_t *cont = lv_obj_create(parent);
	lv_obj_add_style(cont, &style_ManualBtn, LV_PART_MAIN);
	if(!index){
		lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, 10, 0);
		lv_obj_add_event_cb(cont, btnUnlock_event_cb, LV_EVENT_CLICKED, NULL);
	}
	else{
		_settingUI._ScanObj = cont;
		lv_obj_align(cont, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
		lv_obj_add_event_cb(cont, btnPztScan_event_cb, LV_EVENT_CLICKED, NULL);
		//更新扫描初始化状态
		if(!(_settingUI.iSwitchs & (1 << BIT_SCAN)))
			lv_obj_set_style_bg_color(cont, lv_color_hex(0xd74047), LV_PART_MAIN);	
	}
	lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_t *Icon = lv_label_create(cont);
	lv_obj_set_style_text_color(Icon, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(Icon, &font_symbol_20, LV_PART_MAIN);
	lv_label_set_text(Icon, btn_names[index]);
	lv_obj_align(Icon, LV_ALIGN_CENTER, 0, 0);
}



/***************************************************************
* 
*****************************************************************/
void Task_cmdTimeOutSend(lv_timer_t* timer)
{
	enc_flewChk_t *pTag = (enc_flewChk_t *)timer->user_data;

	if(++pTag->idleCnt > IDLE_OVERFLOW_TIME)
	{
		pTag->idleCnt = 0;
		if(pTag->bIsDataSend == false)
		{
			pTag->bIsDataSend = true;
			//对值进行判断后 再决定发送
			spinbox_judge_val(pTag->targetInx, pTag->bIsIncEnabled);
		}
	}
}



void viewGrp_SubCreate(lv_obj_t* parent, viewInfo_t* info, uint8_t i)
{
    lv_obj_t* bar_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(bar_cont);
    lv_obj_set_size(bar_cont, 190, 20);

	//温度/电流/积分蓝色图标
    lv_obj_t* itemLabel = lv_label_create(bar_cont);
    lv_obj_set_style_text_font(itemLabel, &font_symbol_20, 0);
    lv_obj_set_style_text_color(itemLabel, lv_color_hex(0x5dc2f8), 0);
	lv_label_set_text(itemLabel, view_icons[i]);
	lv_obj_align(itemLabel, LV_ALIGN_LEFT_MID, 8, 0);

	lv_obj_t* bar = lv_bar_create(bar_cont);
	lv_obj_add_style(bar, &style_Info, LV_PART_INDICATOR);			 //进度条样式蓝->红 渐变
	lv_obj_set_style_bg_color(bar, lv_color_hex(0xd2d2d6), LV_PART_MAIN); //进度条背景色
	lv_obj_set_size(bar, 100, 15);
	if(i == 0)
		lv_bar_set_range(bar, 0, 200);	//200mA
	else
		lv_bar_set_range(bar, 0, 100);
	lv_obj_align_to(bar, itemLabel, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	lv_bar_set_value(bar, 0, LV_ANIM_ON);
	info->Bar_Main = bar;
    //正常或告警标志
	lv_obj_t* warnLabel = lv_label_create(bar_cont);
	lv_obj_set_style_text_color(warnLabel, lv_color_hex(0xf3b560), 0);
	lv_obj_set_style_text_font(warnLabel, &lv_font_montserrat_14, LV_PART_MAIN);	
	lv_label_set_text(warnLabel, "0.00");
	lv_obj_align_to(warnLabel, bar, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	info->Bar_icon = warnLabel;
	info->cont = bar_cont;
}



void viewGrp_MainCreate(lv_obj_t* parent)
{
	lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN_WRAP);
	lv_obj_set_flex_align(
		parent,
		LV_FLEX_ALIGN_SPACE_EVENLY,
		LV_FLEX_ALIGN_CENTER,
		LV_FLEX_ALIGN_CENTER
	);
	lv_obj_set_style_pad_row(parent, 5, LV_PART_MAIN); 	//设置各item之间的行间距
	const char* TextList[3] =
	{
		"电流(mA)",
		"温度(℃)",
		"积分",
	};
	//监控按钮		
	lv_obj_t *monitor_cont = lv_obj_create(parent);
	lv_obj_remove_style_all(monitor_cont);
	lv_obj_set_size(monitor_cont, 100, 30);
	lv_obj_add_flag(monitor_cont, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(monitor_cont, btnSample_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *monitorIcon = lv_label_create(monitor_cont);
	lv_obj_add_style(monitorIcon, &style_tabIcon, LV_PART_MAIN);
	lv_label_set_text(monitorIcon, MY_ICON_MONITOR);
	lv_obj_align(monitorIcon, LV_ALIGN_CENTER, 0, 0);
		
	for (int i = 0; i < ARRAY_SIZE(_settingUI.labelGrp); i++)
	{
		lv_obj_t *itemLabel = lv_label_create(parent);
		lv_obj_set_style_text_color(itemLabel, lv_color_white(), 0);
		lv_label_set_text(itemLabel, TextList[i]);
		viewGrp_SubCreate(parent, &(_settingUI.labelGrp[i]), i);
	}
}



static void tabBtn_event_cb(lv_event_t * e)
{
	bool isAdmin = *((bool *)(lv_event_get_user_data(e)));
	if(isAdmin == Authority_Developer)
	{
		return;
	}
	lv_obj_t *obj = lv_event_get_target(e);
	uint8_t id = lv_tabview_get_tab_act(lv_obj_get_parent(obj));
	if(id > 1)//除了第1、2个分页
	{
		lv_label_set_text(ui_Dialog.title, "请先登录!");
		Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_Tips_Move);
	}	
}


void setting_tile_init(lv_obj_t *parent)
{
    int i = 0;
	_settingUI.iSwitchs = ui_cfgVal[0].recvDate;
	//默认主题(字体大小，及颜色)
	lv_theme_default_init(NULL, lv_color_hex(0x29a6ac), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
	/*********************************************
	*  定时器 ---- 编码器超时发送
	*********************************************/
	rt_memset((void *)&enc_Chker, 0, sizeof(enc_Chker));
	enc_Chker.enc_flewTimer = lv_timer_create(Task_cmdTimeOutSend, 50, (void *)&enc_Chker);
	lv_timer_pause(enc_Chker.enc_flewTimer);	

	/*********************************************
	* (二) .添加分页菜单
	*********************************************/
	_settingUI._tabCont = lv_tabview_create(parent, LV_DIR_TOP, 40);
	lv_obj_set_style_text_font(_settingUI._tabCont, &font_ch_16, LV_PART_MAIN);
	//分页菜单整体背景色
	lv_obj_set_style_bg_color(_settingUI._tabCont, lv_color_hex(0x20253b), LV_PART_MAIN);
	Gui_AddToIndevGroup(_settingUI._tabCont);
	/*********************************************
	* 2.1  分页1   用户设置窗口
	*********************************************/
	lv_obj_t *subTab = lv_tabview_add_tab(_settingUI._tabCont, "       用户");
	//客户分页按钮配图
	lv_obj_t *tabIcon = lv_label_create(parent);
	lv_obj_add_style(tabIcon, &style_tabIcon, LV_PART_MAIN);
	lv_label_set_text(tabIcon, MY_ICON_LOGIN2);
	lv_obj_set_pos(tabIcon, 30, 10);	
	/*********************************************
	* 2.2 分页1   主视图框，显示部分采集内容
	*********************************************/
	lv_obj_t *view_root = lv_obj_create(subTab);
	lv_obj_set_size(view_root, 195, 200);
	lv_obj_add_style(view_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(view_root, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_clear_flag(view_root, LV_OBJ_FLAG_SCROLLABLE);
	viewGrp_MainCreate(view_root);	
	/*************************************************************************
	* 2.3 分页1   主控制框，用户可用按钮区域
	**************************************************************************/
	lv_obj_t *user_root = lv_obj_create(subTab);
	lv_obj_set_size(user_root, 240, 200);
	lv_obj_add_style(user_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(user_root, LV_ALIGN_RIGHT_MID, 0, 0);
	lv_obj_clear_flag(user_root, LV_OBJ_FLAG_SCROLLABLE);
	//2个客户可用按钮
	for(i = 0;i < 2; i++)
	{
    	_settingUI._mods_sp[i] = switchLock_create(user_root, i, NULL);
	}
	//2个手动按钮       解锁/找峰
	for(i = 0; i < 2; i++)
	{
		Func_ManualBtnInit(user_root, i);
	}	
	/**************************************************************************
	* 3.1 分页2 开发者控制界面
	***************************************************************************/
	subTab = lv_tabview_add_tab(_settingUI._tabCont, "    参数");
	lv_obj_set_flex_flow(subTab, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_add_style(subTab, &style_tabContent, LV_PART_MAIN);	
	//分页2按钮配图
	tabIcon = lv_label_create(parent);
	lv_obj_add_style(tabIcon, &style_tabIcon, LV_PART_MAIN);
	lv_label_set_text(tabIcon, MY_ICON_ENRTY_SETTING);
	lv_obj_set_pos(tabIcon, 140, 10);
	for(i = 0; i < DEVELOP_ITEM_NUMS_END; i++)
	{
    	_settingUI._mods[i] = subTab_create(subTab, i, ENUM_TYPE_TXT, NULL);
	}
	/**************************************************************************
	* 3. 分页3 开发者控制界面
	***************************************************************************/
	subTab = lv_tabview_add_tab(_settingUI._tabCont, "  开发者");
	lv_obj_set_flex_flow(subTab, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_add_style(subTab, &style_tabContent, LV_PART_MAIN);
	//分页3按钮配图
	tabIcon = lv_label_create(parent);
	lv_obj_add_style(tabIcon, &style_tabIcon, LV_PART_MAIN);
	lv_label_set_text(tabIcon, MY_ICON_DEVELOP);
	lv_obj_set_pos(tabIcon, 253, 10);
	for(;i < PARAM_ITEM_NUMS_END; i++)
	{
    	_settingUI._mods[i] = subTab_create(subTab, i, ENUM_TYPE_TXT, NULL);
	}
	/**************************************************************************
	* 4. 分页4 开关控制界面
	***************************************************************************/
	subTab = lv_tabview_add_tab(_settingUI._tabCont, "  开关");
	tabIcon = lv_label_create(parent);
	lv_obj_add_style(tabIcon, &style_tabIcon, LV_PART_MAIN);
	lv_label_set_text(tabIcon, MY_ICON_SWITCH);
	lv_obj_set_pos(tabIcon, 380, 10);

	lv_obj_t *sw_root = lv_obj_create(subTab);
	lv_obj_set_size(sw_root, LV_HOR_RES - 50, 120);
	lv_obj_add_style(sw_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(sw_root, LV_ALIGN_CENTER, 0, 0);
	//框内开关布局
	lv_obj_clear_flag(sw_root, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_flex_flow(sw_root, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_ver(sw_root, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(sw_root, -8, LV_PART_MAIN);
	for(i = 0;i < RELAY_NUMS; i++)
	{
    	_settingUI._mods_sw[i] = subTab_create(sw_root, i, ENUM_TYPE_SW, NULL);
	}
	//tabview 整体风格设定
	lv_obj_t* tab_btns = lv_tabview_get_tab_btns(_settingUI._tabCont);
    lv_obj_set_style_radius(tab_btns, 20, LV_PART_MAIN);
	lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0x3b4066), LV_PART_MAIN);
	lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0x606583), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(0xb5b6bc), LV_PART_MAIN);
	lv_obj_set_style_text_color(tab_btns, lv_color_white(), LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_border_color(tab_btns, lv_color_hex(0x29a6ac), LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_add_event_cb(tab_btns, tabBtn_event_cb, LV_EVENT_CLICKED, (void *)&_settingUI.bIsAdmin);
}


void setting_tile_exit(void)
{	
	int i;
	for(i = 0; i < PARAM_ITEM_NUMS_END; i++)
	{
		rt_free(_settingUI._mods[i]);			
	}
	for(i = 0; i < RELAY_NUMS; i++)
	{
		rt_free(_settingUI._mods_sw[i]);				
	}
	for(i = 0; i < 2; i++)
	{
		rt_free(_settingUI._mods_sp[i]);						
	}
	if(enc_Chker.enc_flewTimer != NULL){
		lv_timer_pause(enc_Chker.enc_flewTimer);
		lv_timer_del(enc_Chker.enc_flewTimer);
	}
}



void Gui_settingInit(lv_obj_t* root)
{
    /*桌面TileView创建，实现桌面左右滑动的效果*/
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t* sample_tile = lv_tileview_add_tile(root, 0, 0, LV_DIR_RIGHT);
    sample_tile_init(sample_tile);
    lv_obj_t* setting_tile = lv_tileview_add_tile(root, 1, 0, LV_DIR_LEFT);
    setting_tile_init(setting_tile);
    /*设置主页面为时间所在页*/
    lv_obj_set_tile(root, setting_tile, LV_ANIM_OFF);	
	//发送第一帧配置数据更新
	Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, MAX_CONFIG_NUM * 2, E_Modbus_Read, 0);
}

void Gui_settingOnFocus(lv_obj_t* root)
{
	return;
}

void Gui_settingExit(lv_obj_t* root)
{
	setting_tile_exit();
	sample_tile_exit();
	return;
}

