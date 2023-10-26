#include "mod_trans.h"
#include "lvgl.h"
#include "lv_port_indev.h"

struct _Ta_Setting _taUI;

#include "ui_utils.h"

const char* Ta_list[TA_T_NUMS_END] = {
    "电流设置(uA)",
	"电流工作点(uA)",		
	"电流告警值(uA)",	
	"温度设置(℃)",
	"温度工作点(℃)",
	"温度最小值(℃)",	
	"温度最大值(℃)",	
};

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
		ui_taVal[index].recvDate = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
		Gui_SendMessge(uart_mq, 
						MODBUS_TA_CFG_ADDR + 2 * index, 
						2, 
						E_Modbus_TA_Write, 
						ui_taVal[index].recvDate);		
    }
}

static void param_dec_event_cb(lv_event_t * e)
{
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
		lv_spinbox_decrement(_taUI._mods[index]->_mObj);
		ui_taVal[index].recvDate = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
		Gui_SendMessge(uart_mq, 
						MODBUS_TA_CFG_ADDR + 2 * index, 
						2, 
						E_Modbus_TA_Write, 
						ui_taVal[index].recvDate);
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
	
	lv_obj_t * btn = spinBtn_style_init(t_objBox, param_inc_event_cb);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
	
	btn = spinBtn_style_init(t_objBox, param_dec_event_cb);
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
		if(inx >= TA_I_NUMS_END){
			t_tabBox->_attr.bHasDot = true;
			t_tabBox->_attr.range_max = 999999;
		}else{
			t_tabBox->_attr.bHasDot = false;
			t_tabBox->_attr.range_max = 5000000;
		}
		t_tabBox->_attr._initVal = ui_taVal[inx].recvDate; //初始化值ui_taVal[0~6]
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
		lv_bar_set_range(bar, 0, 5);	//5A
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
	const char* TextList[4] =
	{
		"采样电流(A)",
		"设置电流(A)",
		"采样温度(℃)",
		"设置温度(℃)",
	};
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
		for(int i = 0; i <= 2; i++)
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
	lv_bar_set_value(info->Bar_Main, (index > 1)?(tmp / 1000):(tmp / 1000000), LV_ANIM_ON);
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
	//if(ret < 0)
	//	lv_label_set_text(info->Bar_icon, LV_SYMBOL_WARNING);
	//else
		lv_label_set_text_fmt(info->Bar_icon, "%.2f", (index > 1)?((float)tmp / 1000):((float)tmp / 1000000));
}


static void btn_exit_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{
		uint8_t id = PAGE_MAIN;
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
			lv_label_set_text(ui_Dialog.title, "是否保存参数?");
			Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_TaSave);
		}
	}			
}

void TA_TaskUpdate(lv_timer_t* timer)
{
	int i;

	if(RT_EOK == rt_sem_trytake(&g_tErrSem))
	{
		rt_kprintf("ta_flush_thread \n");
		for(i = 0; i < TA_T_NUMS_END; i++)
		{
			spinbox_flush_val(_taUI._mods[i]->_mObj, ui_taVal[i].recvDate);
		}
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
	lv_obj_set_style_text_font(root, &font_ch_16, LV_PART_MAIN);
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
	* 1. 电流设置区(分页1)
	**************************************************************************/
    lv_obj_t* current_tile = lv_tileview_add_tile(tvcont, 0, 0, LV_DIR_RIGHT);
	lv_obj_set_flex_flow(current_tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(current_tile, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(current_tile, 0, LV_PART_MAIN);
	for(;i < TA_I_NUMS_END; i++)
	{
    	_taUI._mods[i] = TA_SubCreate(current_tile, i); 
	}
	/*************************************************************************
	* 2. 温度设置区(分页2)
	**************************************************************************/	
  	lv_obj_t* temper_tile = lv_tileview_add_tile(tvcont, 1, 0, LV_DIR_RIGHT | LV_DIR_LEFT);
	lv_obj_set_flex_flow(temper_tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(temper_tile, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(temper_tile, 0, LV_PART_MAIN);
	for(;i < TA_T_NUMS_END; i++)
	{
    	_taUI._mods[i] = TA_SubCreate(temper_tile, i);
	}
	/*************************************************************************
	* 3. 保存设置区(分页3)
	**************************************************************************/		
  	lv_obj_t* other_tile = lv_tileview_add_tile(tvcont, 2, 0, LV_DIR_LEFT);
	lv_obj_t * save_btn = lv_btn_create(other_tile);
	lv_obj_set_size(save_btn, 60, 50);
	lv_obj_set_style_text_font(save_btn, &font_symbol_32,  LV_PART_MAIN);
	lv_obj_set_style_bg_img_src(save_btn, MY_SYMBOL_SAVE, 0);
	lv_obj_set_style_radius(save_btn, 20, LV_PART_MAIN);
	lv_obj_add_event_cb(save_btn, btn_save_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_align(save_btn, LV_ALIGN_CENTER, -50, 0);
	
	lv_obj_t * Exit_btn = lv_btn_create(other_tile);
	lv_obj_set_size(Exit_btn, 60, 50);
	lv_obj_set_style_text_font(Exit_btn, &font_symbol_32,  LV_PART_MAIN);
	lv_obj_set_style_bg_img_src(Exit_btn, MY_SYMBOL_EXIT, 0);
	lv_obj_set_style_radius(Exit_btn, 20, LV_PART_MAIN);	
	lv_obj_add_event_cb(Exit_btn, btn_exit_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_align(Exit_btn, LV_ALIGN_CENTER, 50, 0);
	lv_obj_set_tile(tvcont, current_tile, LV_ANIM_OFF);
	/*************************************************************************
	* 1. 分页器
	**************************************************************************/
	lv_obj_t *tvBtn_cont = lv_obj_create(user_root);
	lv_obj_remove_style_all(tvBtn_cont);	
	lv_obj_set_size(tvBtn_cont, 200, 30);
	lv_obj_align_to(tvBtn_cont, tvcont, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);	
	lv_obj_clear_flag(tvBtn_cont, LV_OBJ_FLAG_SCROLLABLE);
	for(i = -1; i <= 1; i++)
	{
		tvBtn = lv_btn_create(tvBtn_cont);
		lv_obj_set_size(tvBtn, 15, 15);
		lv_obj_set_style_bg_color(tvBtn, lv_color_white(), LV_PART_MAIN);
		lv_obj_set_style_radius(tvBtn, 15, LV_PART_MAIN);
		Gui_setOutlineLight(tvBtn, lv_color_hex(0xff931e), i < 0?true:false);			
		lv_obj_align(tvBtn, LV_ALIGN_CENTER, 25 * i, 0);
	}
	//翻页回调
	lv_obj_add_event_cb(tvcont, tileview_load_event_cb, LV_EVENT_VALUE_CHANGED, (void *)tvBtn_cont);
	Gui_SendMessge(uart_mq, MODBUS_TA_CFG_ADDR, TA_T_NUMS_END * 2, E_Modbus_TA_Read, 0);
	Gui_dialog_Create();
	//modbus每1秒采集一次
	_taUI.sample_timer = lv_timer_create(TA_TaskUpdate, 1000, NULL);
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
	for(int i = 0; i < TA_T_NUMS_END; i++)
	{
		rt_free(_taUI._mods[i]);			
	}
	lv_obj_del(ui_Dialog.cont);
	return;
}

