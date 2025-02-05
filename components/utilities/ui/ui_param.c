
#include "lvgl.h"
#include "lv_port_indev.h"
#include "ui_utils.h"
#if defined(RT_USING_USER_TRANSPORT)
	#include "mod_trans.h"
#endif
#if defined(RT_USING_USER_PARA)
	#include "mod_para.h"
#endif
#if defined(RT_USING_DAC_DEV)
	#include "mod_dac.h"
#endif
#include "application.h"
#include "spi_ad5541.h"

struct _Ta_Setting _taUI;


#if defined(RT_USING_USER_TRANSPORT)

#if defined(USING_ENGLISH_VERSION)
#if defined(USING_HVPZT_SCAN)
const char* Ta_list[TA_HvPzt_NUMS_END] = {
	"Current Setpoint(mA)",
	"Current Reference Setpoint(mA)",		
	"Current Limit(mA)",	
	"TEMP Setpoint(℃)",
	"TEMP Reference Setpoint(℃)",
	"TEMP Min Limit(℃)",	
	"TEMP Max Limit(℃)",	
	"Scan Bias(V)",	
	"Scan Amp(V)",
	"Scan Freq(HZ)",	
};
#else
const char* Ta_list[TA_HvPzt_NUMS_END] = {
  	"Current UserSet (mA)",
	"Current workPoint(mA)",		
	"Current Max(mA)",	
	"Temper UserSet(℃)",
	"Temper workPoint(℃)",
	"Temper Min()",	
	"Temper Max(℃)",	
};

#endif

#else

#if defined(USING_HVPZT_SCAN)
const char* Ta_list[TA_HvPzt_NUMS_END] = {
	"电流设置(mA)",
	"电流工作点(mA)",		
	"电流最大值(mA)",	
	"温度设置(℃)",
	"温度工作点(℃)",
	"温度最小值(℃)",	
	"温度最大值(℃)",	
	"扫描偏置(V)",	
	"扫描幅值(V)",
	"扫描频率(HZ)",	
};
#else
const char* Ta_list[TA_HvPzt_NUMS_END] = {
	"电流设置(mA)",
	"电流工作点(mA)",		
	"电流最大值(mA)",
	/*******************/
	"温度设置(℃)",
	"温度工作点(℃)",
	"温度最小值(℃)",	
	"温度最大值(℃)",
};
#endif

#endif


const char* sample_icons[4] = {
	MY_ICON_CURRENT,
	MY_ICON_CURRENT,		
	MY_ICON_TEMPER,
	MY_ICON_TEMPER,
};



static void param_inc_event_cb(lv_event_t * e)
{
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
	    lv_spinbox_increment(_taUI._mods[index]->_mObj);
#if defined(USING_HVPZT_SCAN)
		if(index <= Item_T_Max){
			ui_taVal[index].recvDate = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
			Gui_SendMessge(uart_mq, 
							MODBUS_TA_CFG_ADDR + 2 * index, 
							2, 
							E_Modbus_TA_Write, 
							ui_taVal[index].recvDate);
		}else{
			Dac_Para *sPara =  (Dac_Para *)e->user_data;
			sPara->params[index - ScanItem_bias] = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
			if(sPara->chipVal.scanState > State_In_ScanClose)
				rt_sem_release(&param_Sem);	
			else{
				M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");
				switch(index)
				{
					case ScanItem_bias:
						pDacChip->_SetWaveFunc(pDacChip, ChipID_HvScan_Bias, HvFunc_Bias2RegVal(sPara->params[index - ScanItem_bias]));	
					break;
					default:
					break;
				}			
			}
		}
#else
		ui_taVal[index].recvDate = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
		Gui_SendMessge(uart_mq, 
						MODBUS_TA_CFG_ADDR + 2 * index, 
						2, 
						E_Modbus_TA_Write, 
						ui_taVal[index].recvDate);
#endif
    }
}

static void param_dec_event_cb(lv_event_t * e)
{
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
		lv_spinbox_decrement(_taUI._mods[index]->_mObj);
#if defined(USING_HVPZT_SCAN)	
		if(index <= Item_T_Max){
			ui_taVal[index].recvDate = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
			Gui_SendMessge(uart_mq, 
							MODBUS_TA_CFG_ADDR + 2 * index, 
							2, 
							E_Modbus_TA_Write, 
							ui_taVal[index].recvDate);
		}else{
			Dac_Para *sPara =  (Dac_Para *)e->user_data;
			sPara->params[index - ScanItem_bias] = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
			if(sPara->chipVal.scanState > State_In_ScanClose)
				rt_sem_release(&param_Sem); 
			else{
				M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");
				switch(index)
				{
					case ScanItem_bias:
						pDacChip->_SetWaveFunc(pDacChip, ChipID_HvScan_Bias, HvFunc_Bias2RegVal(sPara->params[index - ScanItem_bias])); 
					break;
					default:
					break;
				}			
			}
		}
#else
		ui_taVal[index].recvDate = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
		Gui_SendMessge(uart_mq, 
						MODBUS_TA_CFG_ADDR + 2 * index, 
						2, 
						E_Modbus_TA_Write, 
						ui_taVal[index].recvDate);
#endif

    }
}

static void parambox_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
		lv_group_focus_obj(obj);
		lv_obj_add_state(obj, LV_STATE_FOCUSED);
		lv_group_set_editing(lv_group_get_default(), true);	
    }
}

void paramBox_style_init(tab_module_t* t_objBox)
{
	spinContent_style_init(t_objBox, &Ta_list[0], parambox_event_cb);
#if defined(USING_HVPZT_SCAN)	
	lv_obj_t * btn = spinBtn_style_init(t_objBox, param_inc_event_cb, (void *)&_g_ParasPool._uScanPara);
#else
	lv_obj_t * btn = spinBtn_style_init(t_objBox, param_inc_event_cb, NULL);	
#endif
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
	
#if defined(USING_HVPZT_SCAN)		
	btn = spinBtn_style_init(t_objBox, param_dec_event_cb, (void *)&_g_ParasPool._uScanPara);
#else
	btn = spinBtn_style_init(t_objBox, param_dec_event_cb, NULL);	
#endif	

	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_LEFT_MID, -8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
}


tab_module_t *TA_SubCreate(lv_obj_t * parent, uint8_t inx)
{
    tab_module_t* t_tabBox = (tab_module_t *)rt_malloc(sizeof(tab_module_t));
    if (t_tabBox != NULL)
    {
		t_tabBox->_attr.itemIndex = inx;
		t_tabBox->_attr.range_min = 0;
		//精确到小数点后3位
		switch(inx)
		{
			case Item_I_user:
			case Item_I_WorkPoint:
			case Item_I_Max:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.range_max = 275000; 
			break;
			case Item_T_user:
			case Item_T_WorkPoint:
			case Item_T_Min:
			case Item_T_Max:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.range_max = 60000;		
			break;
#if defined(USING_HVPZT_SCAN)			
			//最大100(V)
			case ScanItem_bias:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.range_max = 100000;
			break;
			//最大50(V)			
			case ScanItem_amp:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.range_min = 1000;
				t_tabBox->_attr.range_max = 100000;
			break;
			case ScanItem_freq:
				t_tabBox->_attr.bHasDot = false;
				t_tabBox->_attr.range_min = 1;
				t_tabBox->_attr.range_max = 50;
			break;
#endif
		}
#if defined(USING_HVPZT_SCAN)
		//除高压PZT配置项外都是通信, ui_taVal[0~6]
		if(inx <= Item_T_Max) 
			t_tabBox->_attr._initVal = ui_taVal[inx].recvDate;
		else
			t_tabBox->_attr._initVal = _g_ParasPool._uScanPara.params[inx - ScanItem_bias];
#else
		t_tabBox->_attr._initVal = ui_taVal[inx].recvDate;
#endif
        t_tabBox->_root = lv_obj_create(parent);	
		t_tabBox->_mObj = lv_spinbox_create(t_tabBox->_root);
		paramBox_style_init(t_tabBox);	
    }
    return t_tabBox;
}


void sampleGrp_SubCreate(lv_obj_t* parent, viewInfo_t* info, uint8_t i)
{
    lv_obj_t* bar_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(bar_cont);
    lv_obj_set_size(bar_cont, 190, 20);

	//温度/电流/积分蓝色图标
    lv_obj_t* itemLabel = lv_label_create(bar_cont);
    lv_obj_set_style_text_font(itemLabel, &font_symbol_20, 0);
    lv_obj_set_style_text_color(itemLabel, lv_color_hex(0x5dc2f8), 0);
	lv_label_set_text(itemLabel, sample_icons[i]);
	lv_obj_align(itemLabel, LV_ALIGN_LEFT_MID, 8, 0);

	lv_obj_t* bar = lv_bar_create(bar_cont);
	lv_obj_add_style(bar, &style_Info, LV_PART_INDICATOR);			 //进度条样式蓝->红 渐变
	lv_obj_set_style_bg_color(bar, lv_color_hex(0xd2d2d6), LV_PART_MAIN); //进度条背景色
	lv_obj_set_size(bar, 100, 15);
	if(i <= 1)
		lv_bar_set_range(bar, 0, 275);	//5A
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


void sampleGrp_MainCreate(lv_obj_t* parent)
{
	lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN_WRAP);
	lv_obj_set_flex_align(
		parent,
		LV_FLEX_ALIGN_SPACE_EVENLY,
		LV_FLEX_ALIGN_CENTER,
		LV_FLEX_ALIGN_CENTER
	);
	lv_obj_set_style_pad_row(parent, 6, LV_PART_MAIN); 	//设置各item之间的行间距
#if defined(USING_ENGLISH_VERSION)	
	const char* TextList[4] =
	{
		"Current Sample(mA)",
		"Current Reading(mA)",
		"TEMP Sample(℃)",
		"TEMP Reading(℃)",
	};
#else
	const char* TextList[4] =
	{
		"采样电流(mA)",
		"设置电流(mA)",
		"采样温度(℃)",
		"设置温度(℃)",
	};
#endif		
	for (int i = 0; i < 4; i++)
	{
		lv_obj_t *itemLabel = lv_label_create(parent);
		lv_obj_set_style_text_color(itemLabel, lv_color_white(), 0);
		lv_label_set_text(itemLabel, TextList[i]);
		sampleGrp_SubCreate(parent, &_taUI.labelGrp[i], i);
	}
}



static void tileview_load_event_cb(lv_event_t* e)
{
	if(e->code == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *tv_obj = lv_event_get_target(e);
		lv_obj_t *btn_cont = lv_event_get_user_data(e);	
		
		uint8_t index = lv_obj_get_index(lv_tileview_get_tile_act(tv_obj));
		Gui_setOutlineLight(lv_obj_get_child(btn_cont, index), lv_color_hex(0xff931e), true);
		for(int i = 0; i < Page_Index_Max; i++)
		{
			if(i != index)
				Gui_setOutlineLight(lv_obj_get_child(btn_cont, i), lv_color_white(), false);			
		}
	}
}


void viewBar_Update(viewInfo_t *info, uint8_t index)
{
	uint32_t tmp = ui_taVal[TA_SAMPLE_ADDR + index].recvDate;
	if(tmp & 0xF0000000) //负数
		return;

	int32_t warnVal_Max = 0;
	int32_t warnVal_Min = 0;
	int ret = 0;
	//更新进度条
	lv_bar_set_value(info->Bar_Main, tmp / 1000, LV_ANIM_ON);
	switch(index)
	{
		case Item_Ta_Sample_I:	//采样电流
		case Item_Ta_Setting_I:
			 warnVal_Max = lv_spinbox_get_value(_taUI._mods[Item_Ta_Warn_Imax]->_mObj);
			 if(tmp >= warnVal_Max)
			 {
				ret = -1;
			 }
		break;
		case Item_Ta_Sample_T:	//采样温度
		case Item_Ta_Setting_T:
			 warnVal_Max = lv_spinbox_get_value(_taUI._mods[Item_Ta_Warn_Tmax]->_mObj);
			 warnVal_Min = lv_spinbox_get_value(_taUI._mods[Item_Ta_Warn_Tmin]->_mObj);
			 if(tmp >= warnVal_Max || tmp < warnVal_Min)
			 {
				ret = -1;
			 }			
		break;
		default:
		break;
	}
#if defined(UI_USING_MOD_WARN)	
	if(ret < 0)
		lv_label_set_text(info->Bar_icon, LV_SYMBOL_WARNING);
	else
#endif		
		lv_label_set_text_fmt(info->Bar_icon, "%.2f", (float)tmp / 1000);
}


static void btn_exit_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{
		uint8_t id = PAGE_MAIN;
		lv_timer_pause(_taUI.sample_timer);	
#if defined(USING_HVPZT_SCAN)
		Dac_Para *sPara =  (Dac_Para *)event->user_data;
		//退出界面关闭扫描
		if(sPara->chipVal.scanState == Power_Proc_On)
		{
			sPara->chipVal.scanState = Power_Proc_Off;
			rt_sem_release(&param_Sem);	
		}
#endif
		lv_group_set_editing(lv_group_get_default(), false);
		lv_group_remove_all_objs(lv_group_get_default());
		anim_reback = 0;
		Gui_PageCallback((void *)&id, 1);	
	}
}

static void btn_save_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{
		if(ui_Dialog.cont != NULL)
		{
#if defined(USING_ENGLISH_VERSION)	
			lv_label_set_text(ui_Dialog.title, "Are you sure to save?");
#else
			lv_label_set_text(ui_Dialog.title, "是否保存参数?");
#endif
			Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_TaSave);
		}
	}			
}

#if defined(USING_HVPZT_SCAN)
static void btn_HvPzt_cb(lv_event_t* event)
{
	Dac_Para *sPara =  (Dac_Para *)event->user_data;
	if(event->code == LV_EVENT_CLICKED)
	{
		if(ui_Dialog.cont != NULL)
		{
#if defined(USING_ENGLISH_VERSION)	
			lv_label_set_text(ui_Dialog.title, sPara->chipVal.scanState == Power_Proc_Off?"Are you sure to open Scan?":"Are you sure to close Scan?");
#else		
			lv_label_set_text(ui_Dialog.title, sPara->chipVal.scanState == Power_Proc_Off?"是否开启扫描?":"是否关闭扫描?");
#endif
			Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_Power);
		}
	}
	else if(event->code == LV_EVENT_VALUE_CHANGED)
	{	
		if(sPara->chipVal.scanState == Power_Proc_Off)
		{
			//开启扫描(绿色), 停止刷新采样
			sPara->chipVal.scanState = Power_Proc_On;
			lv_timer_pause(_taUI.sample_timer);
			lv_obj_set_style_bg_color(event->target, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
			rt_sem_release(&scanSw_Sem);
		}
		else{
			//关闭扫描(红色),恢复刷新采样
			sPara->chipVal.scanState = Power_Proc_Off;
			lv_timer_resume(_taUI.sample_timer);
			lv_obj_set_style_bg_color(event->target, lv_color_hex(0xd74047), LV_PART_MAIN);
			rt_sem_release(&param_Sem);
		}			
	}	
}
#endif

static void btn_Switch_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{	
		if((_taUI.opts & (1 << bit_PowerSw)) == Power_Proc_Off)
		{
			//开启继电器(绿色)
			_taUI.opts |= (1 << bit_PowerSw);
			lv_obj_set_style_bg_color(event->target, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
		}
		else{
			//关闭继电器(红色)
			_taUI.opts &= ~(1 << bit_PowerSw);
			lv_obj_set_style_bg_color(event->target, lv_color_hex(0xd74047), LV_PART_MAIN);
		}
		//开关电流都降为0
		lv_spinbox_set_value(_taUI._mods[0]->_mObj, 0);
		Gui_SendMessge(uart_mq, MODBUS_TA_OPTS_ADDR, 2, E_Modbus_TA_Write, _taUI.opts);
	}	
}


void TA_TaskUpdate(lv_timer_t* timer)
{
	int i;

	if(RT_EOK == rt_sem_trytake(&g_tErrSem))
	{
		for(i = 0; i < TA_T_NUMS_END; i++)
		{
			spinbox_flush_val(_taUI._mods[i]->_mObj, ui_taVal[i].recvDate);
		}
		_taUI.opts = ui_taVal[TA_SAMPLE_ADDR - 1].recvDate;
		lv_obj_t *sw_btn = (lv_obj_t *)timer->user_data;
		//更新激光器电源初始化状态
		if(_taUI.opts & (1 << bit_PowerSw))
			lv_obj_set_style_bg_color(sw_btn, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
		else
			lv_obj_set_style_bg_color(sw_btn, lv_color_hex(0xd74047), LV_PART_MAIN);	
		return;
	}
	for (i = 0; i < ARRAY_SIZE(_taUI.labelGrp); i++)
	{
		viewBar_Update(&(_taUI.labelGrp[i]), i);
	}
	//modbus更新采样数据
	Gui_SendMessge(uart_mq, MODBUS_TA_SAMPLE_ADDR, MODBUS_TA_SAMPLE_CNT * 2, E_Modbus_TA_Read, 0);
}


void Gui_paramInit(lv_obj_t* root)
{
	int i = 0;
	lv_obj_t *tvBtn;

	lv_theme_default_init(NULL, lv_color_hex(0x29a6ac), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
	lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(root, 0, LV_PART_MAIN);
#if !defined(USING_ENGLISH_VERSION)	
	lv_obj_set_style_text_font(root, &font_ch_16, LV_PART_MAIN);
#else
	lv_obj_set_style_text_font(root, &font_tw_14, LV_PART_MAIN);
#endif
	lv_obj_set_style_bg_color(root, lv_color_hex(0x20253b), LV_PART_MAIN);
	_taUI._taCont = root;
	Gui_AddToIndevGroup(root);
	/*************************************************************************
	* (一) 采样显示区
	**************************************************************************/
	lv_obj_t *view_root = lv_obj_create(root);
	lv_obj_set_size(view_root, 195, 240);
	lv_obj_add_style(view_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(view_root, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_clear_flag(view_root, LV_OBJ_FLAG_SCROLLABLE);
	sampleGrp_MainCreate(view_root);
	/*************************************************************************
	* (二) 用户设置区(滑动分页)
	**************************************************************************/
	lv_obj_t *user_root = lv_obj_create(root);
	lv_obj_set_size(user_root, 240, 240);
	lv_obj_add_style(user_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(user_root, LV_ALIGN_RIGHT_MID, 0, 0);
	lv_obj_clear_flag(user_root, LV_OBJ_FLAG_SCROLLABLE);
	
	lv_obj_t *tvcont = lv_tileview_create(user_root);
	lv_obj_remove_style_all(tvcont);
	lv_obj_set_size(tvcont, 240, 190);
	lv_obj_align(tvcont, LV_ALIGN_TOP_MID, 0, 0);	
	lv_obj_set_scrollbar_mode(tvcont, LV_SCROLLBAR_MODE_OFF);

	/*************************************************************************
	* 1. 保存设置区(分页1)
	**************************************************************************/		
  	lv_obj_t *other_tile = lv_tileview_add_tile(tvcont, Page_Index_BtnArea, 0, LV_DIR_RIGHT);
	/*************************************************************
	* 板内集成高压扫描模块，不需要通讯，默认退出配置界面就关闭
	**************************************************************/
	//扫描按钮

	lv_obj_t *HvPzt_btn = lv_btn_create(other_tile);
	lv_obj_set_size(HvPzt_btn, 60, 50);
	lv_obj_set_style_text_font(HvPzt_btn, &font_symbol_32,  LV_PART_MAIN);
	lv_obj_set_style_bg_img_src(HvPzt_btn, MY_SYMBOL_SUMMARY, 0);

	lv_obj_set_style_radius(HvPzt_btn, 20, LV_PART_MAIN);
#if defined(USING_HVPZT_SCAN)	
	//每次退出界面均关闭扫描，所以默认是关闭状态（红色）
	lv_obj_set_style_bg_color(HvPzt_btn, lv_color_hex(0xd74047), LV_PART_MAIN);
	lv_obj_add_event_cb(HvPzt_btn, btn_HvPzt_cb, LV_EVENT_CLICKED, (void *)&_g_ParasPool._uScanPara);
	lv_obj_add_event_cb(HvPzt_btn, btn_HvPzt_cb, LV_EVENT_VALUE_CHANGED, (void *)&_g_ParasPool._uScanPara);
#endif
	lv_obj_align(HvPzt_btn, LV_ALIGN_TOP_MID, -40, 30);
	_taUI.scanBtn = HvPzt_btn;
	//电源按钮
	lv_obj_t *sw_btn = lv_btn_create(other_tile);
	lv_obj_set_size(sw_btn, 60, 50);
	lv_obj_set_style_text_font(sw_btn, &font_symbol_20,  LV_PART_MAIN);
	lv_obj_set_style_bg_img_src(sw_btn, MY_ICON_POWER, 0);
	lv_obj_set_style_radius(sw_btn, 20, LV_PART_MAIN);
	lv_obj_add_event_cb(sw_btn, btn_Switch_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_align(sw_btn, LV_ALIGN_TOP_MID, 40, 30);
	//保存按钮
	lv_obj_t * save_btn = lv_btn_create(other_tile);
	lv_obj_set_size(save_btn, 60, 50);
	lv_obj_set_style_text_font(save_btn, &font_symbol_32,  LV_PART_MAIN);
	lv_obj_set_style_bg_img_src(save_btn, MY_SYMBOL_SAVE, 0);
	lv_obj_set_style_radius(save_btn, 20, LV_PART_MAIN);
	lv_obj_add_event_cb(save_btn, btn_save_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_align(save_btn, LV_ALIGN_BOTTOM_MID, -40, -30);
	//离开按钮
	lv_obj_t * Exit_btn = lv_btn_create(other_tile);
	lv_obj_set_size(Exit_btn, 60, 50);
	lv_obj_set_style_text_font(Exit_btn, &font_symbol_32,  LV_PART_MAIN);
	lv_obj_set_style_bg_img_src(Exit_btn, MY_SYMBOL_EXIT, 0);
	lv_obj_set_style_radius(Exit_btn, 20, LV_PART_MAIN);	
#if defined(USING_HVPZT_SCAN)		
	lv_obj_add_event_cb(Exit_btn, btn_exit_cb, LV_EVENT_CLICKED, (void *)&_g_ParasPool._uScanPara);
#else
	lv_obj_add_event_cb(Exit_btn, btn_exit_cb, LV_EVENT_CLICKED, NULL);	
#endif
	lv_obj_align(Exit_btn, LV_ALIGN_BOTTOM_MID, 40, -30);	
#if defined(USING_HVPZT_SCAN)	
	/*************************************************************************
	* 2. 高压PZT设置区(分页2)
	**************************************************************************/	
  	lv_obj_t* hvPzt_tile = lv_tileview_add_tile(tvcont, Page_Index_HVScan, 0, LV_DIR_RIGHT | LV_DIR_LEFT);
	lv_obj_set_flex_flow(hvPzt_tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(hvPzt_tile, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(hvPzt_tile, 0, LV_PART_MAIN);
	for(i = TA_T_NUMS_END;i < TA_HvPzt_NUMS_END; i++)
	{
    	_taUI._mods[i] = TA_SubCreate(hvPzt_tile, i);
	}	
#endif
	/*************************************************************************
	* 1. 电流设置区(分页3)
	**************************************************************************/
    lv_obj_t* current_tile = lv_tileview_add_tile(tvcont, Page_Index_I, 0, LV_DIR_RIGHT | LV_DIR_LEFT);
	lv_obj_set_flex_flow(current_tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(current_tile, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(current_tile, 0, LV_PART_MAIN);
	for(i = 0;i < TA_I_NUMS_END; i++)
	{
    	_taUI._mods[i] = TA_SubCreate(current_tile, i); 
	}
	/*************************************************************************
	* 2. 温度设置区(分页4)
	**************************************************************************/	
  	lv_obj_t* temper_tile = lv_tileview_add_tile(tvcont, Page_Index_T, 0, LV_DIR_LEFT);
	lv_obj_set_flex_flow(temper_tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(temper_tile, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(temper_tile, 0, LV_PART_MAIN);
	for(;i < TA_T_NUMS_END; i++)
	{
    	_taUI._mods[i] = TA_SubCreate(temper_tile, i);
	}
	//设置主页为分页1
	lv_obj_set_tile(tvcont, other_tile, LV_ANIM_OFF);
	/*************************************************************************
	* 1. 分页器
	**************************************************************************/
	lv_obj_t *tvBtn_cont = lv_obj_create(user_root);
	lv_obj_remove_style_all(tvBtn_cont);	
	lv_obj_set_size(tvBtn_cont, 200, 30);
	lv_obj_align_to(tvBtn_cont, tvcont, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);	
	lv_obj_clear_flag(tvBtn_cont, LV_OBJ_FLAG_SCROLLABLE);
	for(i = 0; i < Page_Index_Max; i++)
	{
		tvBtn = lv_btn_create(tvBtn_cont);
		lv_obj_set_size(tvBtn, 15, 15);
		lv_obj_set_style_bg_color(tvBtn, lv_color_white(), LV_PART_MAIN);
		lv_obj_set_style_radius(tvBtn, 15, LV_PART_MAIN);
		Gui_setOutlineLight(tvBtn, lv_color_hex(0xff931e), i == 0?true:false);
#if defined(USING_HVPZT_SCAN)
		switch(i)
		{
			case 0:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, -35, 0);
			break;
			case 1:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, -12, 0);
			break;
			case 2:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, 12, 0);
			break;
			case 3:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, 35, 0);
			break;
		}
#else
		switch(i)
		{
			case 0:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, -25, 0);
			break;
			case 1:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, 0, 0);
			break;
			case 2:
				lv_obj_align(tvBtn, LV_ALIGN_CENTER, 25, 0);
			break;
		}
#endif
	}
	//翻页回调
	lv_obj_add_event_cb(tvcont, tileview_load_event_cb, LV_EVENT_VALUE_CHANGED, (void *)tvBtn_cont);
	//第一次发送，获取除了HVPZT外所有数据(电流 + 温度 + options)
	Gui_SendMessge(uart_mq, MODBUS_TA_CFG_ADDR, OPTIONS_NUMS_END * 2, E_Modbus_TA_Read, 0);
	Gui_dialog_Create();
	//modbus每1秒采集一次
	_taUI.sample_timer = lv_timer_create(TA_TaskUpdate, 1000, (void *)sw_btn);
	lv_timer_resume(_taUI.sample_timer);	
}



void Gui_paramOnFocus(lv_obj_t* root)
{
	return;
}

void Gui_paramExit(lv_obj_t* root)
{
	if(_taUI.sample_timer != NULL)
	{
		lv_timer_pause(_taUI.sample_timer);
		lv_timer_del(_taUI.sample_timer);
	}	
	for(int i = 0; i < TA_HvPzt_NUMS_END; i++)
	{
		rt_free(_taUI._mods[i]);			
	}
	lv_obj_del(ui_Dialog.cont);
	return;
}

#endif

