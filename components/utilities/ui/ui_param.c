#include "mod_trans.h"
#include "lvgl.h"
#include "lv_port_indev.h"
#include "ui_utils.h"

#define bit_powerSw		0
#define bit_lightSw		1

struct _Fiber_Setting _fiberUI;


const char* fiber_list[FIBER_NUMS_END] = {
    "电流设置(A)",
	"工作频率(Hz)",		
	"占空比(%)",	
};

const char* btn_icons[4] = {
	MY_SYMBOL_FIBER_ENABLE,
	MY_SYMBOL_LIGHT_OPEN,
	MY_SYMBOL_WORK_MODE,
	MY_SYMBOL_EXIT,
};

char sendStr[50];
const char* e_workMode[3] = {"DGC","DGQ","ANG"};
const char* c_workMode[3] = {"连续模式","准连续模式","模拟模式"};


const char* fiber_Warn_list[FIBER_WARN_NUMS_END] = {
	/********温度下限告警*******/		
    "温度1下限",
    "温度2下限",
    "温度3下限",
	"温度4下限",		
	"温度5下限",
	"温度6下限",
	/********pd告警*******/	
    "PD0上限",
    "PD1上限",
    "PD2上限",
	"PD3上限",		
	"PD0下限",
	"PD1下限",
	"PD2下限",
	"PD3下限",
	/********温度上限告警*******/		
    "温度1上限",
    "温度2上限",
    "温度3上限",
	"温度4上限",		
	"温度5上限",
	"温度6上限",	
	/********其他告警*******/		
    "联锁状态",
    "种子状态",
    "使能状态",
    "发光状态",
	"故障状态",		
	"报警状态",
};

const char* btnTitle[4] = {
	"使能开关",
	"出光使能",
	"工作模式",
	"退出界面",
};

static void btn_power_Switch_cb(lv_event_t* event);
static void btn_light_Switch_cb(lv_event_t* event);
static void btn_mode_Switch_cb(lv_event_t* event);
static void btn_exit_cb(lv_event_t* event);

M_FuncBtnAttr BtnPool[4] =
{
	{-40, 30, 0xd74047, RT_NULL, btn_power_Switch_cb },
	{40, 30, 0xd74047, RT_NULL, btn_light_Switch_cb },
	{-40, -30, 0x29a6ac, RT_NULL, btn_mode_Switch_cb },
	{40, -30, 0x29a6ac, RT_NULL, btn_exit_cb },
};
/***********************************************************************************
*								参数设置区
************************************************************************************/
static void param_inc_event_cb(lv_event_t * e)
{
	int  i;
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
		if(index == _cfg_index_ISet){


		}
			
        lv_spinbox_increment(_fiberUI._mods[index]->_mObj);	
		for(i = 0; i < FIBER_NUMS_END; i++)
			ui_FiberConf.cfg_val[i] = lv_spinbox_get_value(_fiberUI._mods[i]->_mObj);
		
		lv_snprintf(sendStr, 19, "OUT=%04x%05x%03x\r\n", ui_FiberConf.cfg_val[_cfg_index_ISet], 
														 ui_FiberConf.cfg_val[_cfg_index_Freq], 
														 ui_FiberConf.cfg_val[_cfg_index_Duty]);
		FIBER_ATcmd(_cmd_index_set_output, sendStr);

    }
}


static void param_dec_event_cb(lv_event_t * e)
{
	int i;
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
		lv_spinbox_decrement(_fiberUI._mods[index]->_mObj);
		for(i = 0; i < FIBER_NUMS_END; i++)
			ui_FiberConf.cfg_val[i] = lv_spinbox_get_value(_fiberUI._mods[i]->_mObj);
		
		lv_snprintf(sendStr, 19, "OUT=%04x%05x%03x\r\n", ui_FiberConf.cfg_val[_cfg_index_ISet], 
														 ui_FiberConf.cfg_val[_cfg_index_Freq], 
														 ui_FiberConf.cfg_val[_cfg_index_Duty]);
		FIBER_ATcmd(_cmd_index_set_output, sendStr);
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

void paramBox_style_init(ctrl_module_t* t_objBox)
{
	spinContent_style_init(t_objBox, &fiber_list[0], parambox_event_cb);
	
	lv_obj_t * btn = spinBtn_style_init(t_objBox, param_inc_event_cb);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
	
	btn = spinBtn_style_init(t_objBox, param_dec_event_cb);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_LEFT_MID, -8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
}



ctrl_module_t *fiber_SubCreate(lv_obj_t * parent, uint8_t inx)
{
    ctrl_module_t* t_tabBox = (ctrl_module_t *)rt_malloc(sizeof(ctrl_module_t));
    if (t_tabBox != NULL)
    {
		t_tabBox->_attr.itemIndex = inx;
		t_tabBox->_attr.range_min = 0;
		t_tabBox->_attr.bHasDot = false;
		switch(inx)
		{
			case _cfg_index_ISet:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.wholeCnt = 4;
				t_tabBox->_attr.integerCnt = 2;
				t_tabBox->_attr.accuracy = 0.01;
				t_tabBox->_attr.range_max = 9999;
				break;
			case _cfg_index_Freq:
				t_tabBox->_attr.wholeCnt = 6;
				t_tabBox->_attr.integerCnt = 6;	
				t_tabBox->_attr.accuracy = 1.0;
				t_tabBox->_attr.range_max = 100000;
			break;
			case _cfg_index_Duty:
				t_tabBox->_attr.wholeCnt = 4;
				t_tabBox->_attr.integerCnt = 3;	
				t_tabBox->_attr.accuracy = 0.1;
				t_tabBox->_attr.range_max = 100;

			break;
		}
		t_tabBox->_attr._initVal = ui_FiberConf.cfg_val[inx]; //初始化值ui_taVal[0~6]
        t_tabBox->_root = lv_obj_create(parent);	
		t_tabBox->_mObj = lv_spinbox_create(t_tabBox->_root);
		paramBox_style_init(t_tabBox);	
    }
    return t_tabBox;
}




/*************************************************************************************/
warn_module_t* WarnItem_Create(lv_obj_t * parent, uint8_t inx, uint8_t valPos, uint8_t bitPos)
{
    warn_module_t* itemWarn = (warn_module_t *)rt_malloc(sizeof(warn_module_t));

    lv_obj_t* warn_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(warn_cont);
    lv_obj_set_size(warn_cont, 190, 20);

	//左->右： 文字 + 图标（告警或正常）
    lv_obj_t *itemLabel = lv_label_create(warn_cont);
	lv_obj_set_style_text_color(itemLabel, lv_color_white(), LV_PART_MAIN);
	lv_label_set_text(itemLabel, fiber_Warn_list[inx]);
	lv_obj_align(itemLabel, LV_ALIGN_LEFT_MID, 0, 0);

  	lv_obj_t *statIcon = lv_label_create(warn_cont);
	lv_obj_set_style_text_font(statIcon, &lv_font_montserrat_14, LV_PART_MAIN);	
	lv_obj_set_style_text_color(statIcon, lv_color_hex(0xefb02a), LV_PART_MAIN);

	if(ui_FiberConf.warn_val[valPos] & (1 << bitPos))
		lv_label_set_text(statIcon, LV_SYMBOL_WARNING);
	else
		lv_label_set_text(statIcon, LV_SYMBOL_OK);		
	lv_obj_align_to(statIcon, itemLabel, LV_ALIGN_OUT_RIGHT_MID, 50, 0);
	
	itemWarn->bitPos = bitPos;
	itemWarn->valPos = valPos;
	itemWarn->_iconObj = statIcon;
	return itemWarn;
}

/*************************************************************************************/
//滚屏回调
static void tileview_load_event_cb(lv_event_t* e)
{
	if(e->code == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *tv_obj = lv_event_get_target(e);
		lv_obj_t *div_cont = lv_event_get_user_data(e);	
		//获得当前页码
		uint8_t index = lv_obj_get_index(lv_tileview_get_tile_act(tv_obj));
		Gui_setOutlineLight(lv_obj_get_child(div_cont, index), lv_color_hex(0xff931e), true);
		for(int i = 0; i <= 4; i++)
		{
			if(i != index)
				Gui_setOutlineLight(lv_obj_get_child(div_cont, i), lv_color_white(), false);			
		}
	}
}

/*************************************************************************
* 光纤电源开关回调--------check ok
**************************************************************************/

static void btn_power_Switch_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{	
		lv_obj_t *btnObj = lv_event_get_target(event);
		if((ui_FiberConf.switchs & (1 << bit_powerSw)) == 0)
		{
			BtnPool[0].iColor = 0x29a6ac;
			ui_FiberConf.switchs |= 1 << bit_powerSw;		
			FIBER_ATcmd(_cmd_index_set_switch, "ENABLE=100\r\n");
		}else{
			BtnPool[0].iColor = 0xd74047;
			ui_FiberConf.switchs &= ~(1 << bit_powerSw);
			FIBER_ATcmd(_cmd_index_set_switch, "ENABLE=000\r\n");
		}
	}			
}



/*************************************************************************
* 光纤光使能开关回调--------check ok
**************************************************************************/
static void btn_light_Switch_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{	
		lv_obj_t *btnObj = lv_event_get_target(event);
		if((ui_FiberConf.switchs & (1 << bit_lightSw)) == 0)
		{
			BtnPool[1].iColor = 0x29a6ac;
			ui_FiberConf.switchs |= 1 << bit_lightSw;		
			FIBER_ATcmd(_cmd_index_set_switch, "ENABLE=1FF\r\n");
		}else{
			BtnPool[1].iColor = 0xd74047;
			ui_FiberConf.switchs &= ~(1 << bit_lightSw);
			FIBER_ATcmd(_cmd_index_set_switch, "ENABLE=100\r\n");
		}
		lv_obj_set_style_bg_color(btnObj, lv_color_hex(BtnPool[1].iColor), LV_PART_MAIN);
	}			
}


/*************************************************************************
* 光纤工作模式切换--------check ok
**************************************************************************/
static void btn_mode_Switch_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{
		lv_obj_t *title = (lv_obj_t *)lv_event_get_user_data(event);

		if(++ui_FiberConf.mode ==  fiber_mode_max)
		{
			ui_FiberConf.mode = fiber_mode_DGC;
		}		
		lv_label_set_text(title, c_workMode[ui_FiberConf.mode]);
		lv_snprintf(sendStr, 11, "OPMD=%s\r\n", e_workMode[ui_FiberConf.mode]);
		FIBER_ATcmd(_cmd_index_set_mode, sendStr);
	}			
}

/*************************************************************************
* 离开界面--------check ok
**************************************************************************/
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




//定时采集状态、PD值
void stat_TaskUpdate(lv_timer_t* timer)
{
	//刚进入app时的第一次数据交互
	int i;
	if(RT_EOK == rt_sem_trytake(&g_tErrSem))
	{
		//更新当前命令返回的界面
		switch(_fiberUI.sample_inx)
		{
			case _cmd_index_get_I_lv1:
			case _cmd_index_get_I_lv2:
			case _cmd_index_get_PD:					
			break;
			case _cmd_index_get_status:
				for(i = 0;i < FIBER_WARN_NUMS_END; i++)
				{
					if(ui_FiberConf.warn_val[_fiberUI._items[i]->valPos] & (1 << _fiberUI._items[i]->bitPos))
						lv_label_set_text(_fiberUI._items[i]->_iconObj, LV_SYMBOL_WARNING);
					else
						lv_label_set_text(_fiberUI._items[i]->_iconObj, LV_SYMBOL_OK);	
				}				
			break;
			case _cmd_index_get_mode:
				if(BtnPool[2].ctrlObj != RT_NULL)
					lv_label_set_text(BtnPool[2].ctrlObj, c_workMode[ui_FiberConf.mode]);
			break;
			case _cmd_index_get_output:
				if(ui_FiberConf.mode == fiber_mode_DGQ)
				{
					for(i = 0; i < _cfg_index_Max; i++)
					{
						spinbox_flush_val(_fiberUI._mods[i]->_mObj, ui_FiberConf.cfg_val[i]);
					}
				}else if(ui_FiberConf.mode == fiber_mode_DGC){
					spinbox_flush_val(_fiberUI._mods[_cfg_index_ISet]->_mObj, ui_FiberConf.cfg_val[_cfg_index_ISet]);
				}
			break;			
		}
		//更新当前采集命令的限制次数
		if(_pATCmd[_fiberUI.sample_inx].limitCnt > 0)
			_pATCmd[_fiberUI.sample_inx].limitCnt--;	
		//进入下一个可发命令
		do{
			if(++_fiberUI.sample_inx == _cmd_index_get_id)
				_fiberUI.sample_inx = _cmd_index_get_I_lv1;

		}while(!_pATCmd[_fiberUI.sample_inx].limitCnt);
		FIBER_ATcmd(_fiberUI.sample_inx, RT_NULL);		
	}
}




void Gui_paramInit(lv_obj_t* root)
{
	int i = 0;
	lv_obj_t *div;

	ui_FiberConf.mode = fiber_mode_DGC;
	/*************************************************************************
	* (0)设置整体主题
	**************************************************************************/
	lv_theme_default_init(NULL, lv_color_hex(0x29a6ac), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
	lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(root, 0, LV_PART_MAIN);
	lv_obj_set_style_text_font(root, &font_ch_16, LV_PART_MAIN);
	lv_obj_set_style_bg_color(root, lv_color_hex(0x20253b), LV_PART_MAIN);
	_fiberUI._taCont = root;
	Gui_AddToIndevGroup(root);
	/*************************************************************************
	* (一) (滑动分页)（左侧）
	**************************************************************************/
	lv_obj_t *warn_root = lv_obj_create(root);
	lv_obj_set_size(warn_root, 195, 240);
	lv_obj_add_style(warn_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(warn_root, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_clear_flag(warn_root, LV_OBJ_FLAG_SCROLLABLE);	//禁用滚动条
	//设置滑动区域
	lv_obj_t *tvcont = lv_tileview_create(warn_root);
	lv_obj_remove_style_all(tvcont);
	lv_obj_set_size(tvcont, 190, 200);
	lv_obj_align(tvcont, LV_ALIGN_TOP_MID, 0, 0);	
	lv_obj_set_scrollbar_mode(tvcont, LV_SCROLLBAR_MODE_OFF);
	/*************************************************************************
	* 1. 功能按键区(分页1)
	**************************************************************************/		
  	lv_obj_t *btnPage = lv_tileview_add_tile(tvcont, Page_Index_BtnArea, 0, LV_DIR_RIGHT);
	for(i = 0; i < 4; i++)
	{
		lv_obj_t * titleLabel = lv_label_create(btnPage);
		lv_obj_set_style_text_color(titleLabel, lv_color_white(), LV_PART_MAIN);		
		lv_label_set_text(titleLabel, btnTitle[i]);
		if(i < 2)
		{
			lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, (!i)? -38:38, 0);		
		}else{
			lv_obj_align(titleLabel, LV_ALIGN_CENTER, (i == 2)? -38:38, 0);
		}
		_fiberUI.ptrBtn = &BtnPool[i];
		_fiberUI.ptrBtn->ctrlObj = titleLabel;
		lv_obj_t *btnObj = lv_btn_create(btnPage);
		lv_obj_add_style(btnObj, &style_FuncBtn, LV_PART_MAIN);	
		lv_obj_set_style_bg_img_src(btnObj, btn_icons[i], LV_PART_MAIN);
		lv_obj_set_style_bg_color(btnObj, lv_color_hex(_fiberUI.ptrBtn->iColor), LV_PART_MAIN);	
		lv_obj_add_event_cb(btnObj, _fiberUI.ptrBtn->funcHandler, LV_EVENT_CLICKED, (void *)titleLabel);	
		lv_obj_align(btnObj, (i < 2)? LV_ALIGN_TOP_MID : LV_ALIGN_BOTTOM_MID, _fiberUI.ptrBtn->xPos, _fiberUI.ptrBtn->yPos);
	}
	/*************************************************************************
	* 2. 温度下限告警区(分页2)
	**************************************************************************/	
	lv_obj_t* _Page_temperDown = lv_tileview_add_tile(tvcont, Page_Index_WarnTempD, 0, LV_DIR_RIGHT | LV_DIR_LEFT);	
	//纵向布局
	lv_obj_add_style(_Page_temperDown, &style_SubPage, LV_PART_MAIN);

	//垂直滚动， 不显示滚动条，但不禁用
	lv_obj_set_scroll_dir(_Page_temperDown, LV_DIR_VER);
	lv_obj_set_style_bg_opa(_Page_temperDown, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(_Page_temperDown, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);		
	for(i = 0;i < FIBER_WARN_T_DOWNLIMIT_END; i++)
	{
    	_fiberUI._items[i] = WarnItem_Create(_Page_temperDown, i, Item_Warn_Temp_down, i + 2); 
	}
	/*************************************************************************
	* 3. PD上下限告警区(分页3)
	**************************************************************************/	
  	lv_obj_t* _Page_pd = lv_tileview_add_tile(tvcont, Page_Index_WarnPD, 0, LV_DIR_RIGHT | LV_DIR_LEFT);
	lv_obj_add_style(_Page_pd, &style_SubPage, LV_PART_MAIN);
	//垂直滚动
	lv_obj_set_scroll_dir(_Page_pd, LV_DIR_VER);
	lv_obj_set_style_bg_opa(_Page_pd, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(_Page_pd, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);			
	for(;i < FIBER_WARN_PD_NUMS_END; i++)
	{
    	_fiberUI._items[i] = WarnItem_Create(_Page_pd, i, Item_Warn_PdX, i - FIBER_WARN_T_DOWNLIMIT_END);
	}
	/*************************************************************************
	* 4. 温度上限告警区(分页4)
	**************************************************************************/	
  	lv_obj_t* _Page_temperUp = lv_tileview_add_tile(tvcont, Page_Index_WarnTempU, 0, LV_DIR_RIGHT | LV_DIR_LEFT);
	lv_obj_add_style(_Page_temperUp, &style_SubPage, LV_PART_MAIN);

	lv_obj_set_scroll_dir(_Page_temperUp, LV_DIR_VER);
	lv_obj_set_style_bg_opa(_Page_temperUp, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(_Page_temperUp, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);			
	for(;i < FIBER_WARN_T_UPLIMIT_END; i++)
	{
    	_fiberUI._items[i] = WarnItem_Create(_Page_temperUp, i, Item_Warn_Temp_up, i + 2 - FIBER_WARN_PD_NUMS_END); 
	}
	/*************************************************************************
	* 4. 其他告警区(分页5)
	**************************************************************************/	
  	lv_obj_t* _Page_other = lv_tileview_add_tile(tvcont, Page_Index_WarnOther, 0, LV_DIR_LEFT);
	lv_obj_add_style(_Page_other, &style_SubPage, LV_PART_MAIN);
	lv_obj_set_scroll_dir(_Page_other, LV_DIR_VER);
	lv_obj_set_style_bg_opa(_Page_other, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(_Page_other, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);	
	
	for(;i < FIBER_WARN_NUMS_END; i++)
	{
		if(i < FIBER_WARN_NUMS_END - 4)
    		_fiberUI._items[i] = WarnItem_Create(_Page_other, i, Item_Warn_LockStat, 5 + i - FIBER_WARN_T_UPLIMIT_END); 
		else
    		_fiberUI._items[i] = WarnItem_Create(_Page_other, i, Item_Warn_Oher, i - FIBER_WARN_T_UPLIMIT_END - 1 ); 
	}
	/*************************************************************************
	* 5. 分页器
	**************************************************************************/
	lv_obj_t *pageDiv_cont = lv_obj_create(warn_root);
	lv_obj_remove_style_all(pageDiv_cont);	
	lv_obj_set_size(pageDiv_cont, 195, 40);
	lv_obj_align_to(pageDiv_cont, tvcont, LV_ALIGN_OUT_BOTTOM_MID, 0, -5);	
	lv_obj_clear_flag(pageDiv_cont, LV_OBJ_FLAG_SCROLLABLE);
	for(i = -2; i <= 2; i++)
	{
		div = lv_btn_create(pageDiv_cont);
		lv_obj_set_size(div, 12, 12);
		lv_obj_set_style_bg_color(div, lv_color_white(), LV_PART_MAIN);
		lv_obj_set_style_radius(div, 12, LV_PART_MAIN);
		Gui_setOutlineLight(div, lv_color_hex(0xff931e), i == -2?true:false);			
		lv_obj_align(div, LV_ALIGN_CENTER, 25 * i, 0);
	}	
	//翻页回调
	lv_obj_add_event_cb(tvcont, tileview_load_event_cb, LV_EVENT_VALUE_CHANGED, (void *)pageDiv_cont);
	
	/*************************************************************************
	* (二) 设置参数区（右侧）
	**************************************************************************/
	lv_obj_t *user_root = lv_obj_create(root);
	lv_obj_set_size(user_root, 240, 240);
	lv_obj_add_style(user_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(user_root, LV_ALIGN_RIGHT_MID, 0, 0);

	//允许滚动
	lv_obj_add_flag(user_root, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_flex_flow(user_root, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(user_root, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_row(user_root, 0, LV_PART_MAIN);
	for(i= 0;i < FIBER_NUMS_END; i++)
	{
    	_fiberUI._mods[i] = fiber_SubCreate(user_root, i); 
	}
	_fiberUI.sample_inx = _cmd_index_get_I_lv1;
	FIBER_ATcmd(_fiberUI.sample_inx, RT_NULL);	
	//每1秒采集一次	
	_fiberUI.sample_timer = lv_timer_create(stat_TaskUpdate, 1000, NULL);
	lv_timer_resume(_fiberUI.sample_timer);		
}

void Gui_paramOnFocus(lv_obj_t* root)
{
	return;
}


void Gui_paramExit(lv_obj_t* root)
{
	int i;
	
	if(_fiberUI.sample_timer != NULL)
	{
		lv_timer_pause(_fiberUI.sample_timer);
		lv_timer_del(_fiberUI.sample_timer);
	}	
	for(i = 0; i < FIBER_NUMS_END; i++)
	{
		rt_free(_fiberUI._mods[i]);			
	}
	for(i = 0; i < FIBER_WARN_NUMS_END; i++)
	{
		rt_free(_fiberUI._items[i]);			
	}	
}



