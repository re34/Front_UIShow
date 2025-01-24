#include "ui_utils.h"
#include "mod_trans.h"

//extern int dbg_printf(const char *fmt, ...);

#define TOP_WITH			480
#define TOP_HEIGHT			120

#define MTM_GAP             12
/*组件宽度*/
#define MODULE_WIDTH        (TOP_WITH - MTM_GAP * 3) / 2
/*组件高度*/
#define MODULE_HEIGHT       (TOP_HEIGHT - MTM_GAP * 2)

#define SMALL_MODULE_WIDTH  (TOP_WITH - MTM_GAP * 4) / 3

#if defined(USING_SECOND_TEMPER)
const char* list2_icons[MAIN_SAMPLE_NUMS] = {
	MY_ICON_CURRENT,
	MY_ICON_TEMPER,
	MY_ICON_TEMPER,
};

const char* list1_label[MAIN_SAMPLE_NUMS] = 
{
	"Current(mA)",
	"Temper1(℃)",
	"Temper2(℃)",	
};
#else

const char* list2_icons[MAIN_SAMPLE_NUMS] = {
	MY_SYMBOL_FREQ,
	MY_SYMBOL_TEMPER,
};

const char* list1_label[MAIN_SAMPLE_NUMS] = {
	"Current (mA)",
	"Temperature (℃)",
};
#endif

struct _ui_info ui;



void module_set_usage_value(module_t *module)
{
	if((module->mod_sample->recvDate) & 0xF0000000) //负数
		return;
	float value = ((float)(module->mod_sample->recvDate))/ 1000;
    lv_label_set_text_fmt(module->usage_label, "%.3f", value);
    lv_chart_set_next_value(module->usage_chart, module->usage_series, (lv_coord_t)value);
}



void lv_obj_set_opa_scale(lv_obj_t* obj, int16_t opa)
{
    lv_obj_set_style_bg_opa(obj, (lv_opa_t)opa, LV_PART_MAIN);
}



#if defined(USING_SECOND_TEMPER)
void viewGrp_Update(viewInfo_t *info, uint8_t index)
{
	uint32_t tmp = 0;
	int32_t warnVal = 0;
	tmp = ui_sample[DATA_OFFSET + index].recvDate;
	if(tmp & 0xF0000000) //负数
		return;
	//更新进度条
	lv_bar_set_value(info->Bar_Main, tmp / 1000, LV_ANIM_ON);
	if(index == Item_Temper_Lv2)
		warnVal = lv_spinbox_get_value(_settingUI._mods[index + ALARM_OFFSET - 1]->_mObj);
	else
		warnVal = lv_spinbox_get_value(_settingUI._mods[index + ALARM_OFFSET]->_mObj);		
	//采样值正常或无告警采样项正常显示数值
	if(tmp < warnVal)
		lv_label_set_text_fmt(info->Bar_icon, "%.2f", ((float)tmp / 1000));
	else
		lv_label_set_text(info->Bar_icon, LV_SYMBOL_WARNING);
}

#else

void viewGrp_Update(viewInfo_t *info, uint8_t index)
{
	uint32_t tmp = 0;
	if(index <= Item_Temper_Lv1)
		tmp = ui_sample[DATA_OFFSET + index].recvDate;
	else
		tmp = ui_sample[1].recvDate;
	if(tmp & 0xF0000000) //负数
		return;
	//更新进度条
	lv_bar_set_value(info->Bar_Main, tmp / 1000, LV_ANIM_ON);
	int32_t warnVal = lv_spinbox_get_value(_settingUI._mods[index + ALARM_OFFSET]->_mObj);
	if(tmp < warnVal || index > Item_Temper_Lv1)
		lv_label_set_text_fmt(info->Bar_icon, "%.2f", ((float)tmp / 1000));
	else
		lv_label_set_text(info->Bar_icon, LV_SYMBOL_WARNING);
}
#endif


void sample_update(uint8_t lockState)
{
	module_set_usage_value(ui.topInfo.I_module);
		module_set_usage_value(ui.topInfo.T1_module);
#if defined(USING_SECOND_TEMPER)
		module_set_usage_value(ui.topInfo.T2_module);
#endif

	lv_label_set_text_fmt(
		ui.bottomInfo.labelInfoGrp[0].lableValue,
		"%d",
		ui.bottomInfo.labelInfoGrp[0].sample->recvDate
	);
	for(int i = 1; i <= 4; i++)
	{		
		uint32_t val = ui.bottomInfo.labelInfoGrp[i].sample->recvDate;
		if(val & 0xF0000000)
		{
			val = ~val + 1;
			lv_label_set_text_fmt(
				ui.bottomInfo.labelInfoGrp[i].lableValue,
				"-%.3f",
				((float)val) / 1000
			);			
		}else{
			lv_label_set_text_fmt(
				ui.bottomInfo.labelInfoGrp[i].lableValue,
				"%.3f",
				((float)val) / 1000
			);	
		}
	}	
	lv_label_set_text(
		ui.bottomInfo.labelInfoGrp[5].lableValue,
		lockState?"Locked":"Unlock"
	);
	lv_obj_set_style_text_color(
		ui.bottomInfo.labelInfoGrp[5].lableValue, 
		lv_palette_main(lockState? LV_PALETTE_GREEN : LV_PALETTE_RED), 
		0
	);
}


void Info_TaskUpdate(lv_timer_t* timer)
{
	//刚进入app时的第一次数据交互
	if(RT_EOK == rt_sem_trytake(&g_tErrSem))
	{
		int i;
		for(i = 0; i < PARAM_ITEM_NUMS_END; i++)
		{
			spinbox_flush_val(_settingUI._mods[i]->_mObj, ui_cfgVal[1 + i].recvDate);
		}
		for(i = 0; i < (RELAY_NUMS - 1); i++)
		{
			sw_flush_val(_settingUI._mods_sw[i]->_mObj, i, ui_cfgVal[0].recvDate);
		}
		//刷新扫描开关
		if(!(ui_cfgVal[0].recvDate & (1 << BIT_SCAN)))
		{
			lv_obj_set_style_bg_color(_settingUI._ScanObj, lv_color_hex(0xd74047), LV_PART_MAIN);
		}else{
			lv_obj_set_style_bg_color(_settingUI._ScanObj, lv_color_hex(0x3b67b0), LV_PART_MAIN);
		}		
		_settingUI.iSwitchs = ui_cfgVal[0].recvDate;
		return;
	}
	lv_obj_t *tv_obj = lv_obj_get_parent((lv_obj_t *)timer->user_data);
	uint8_t index = lv_obj_get_index(lv_tileview_get_tile_act(tv_obj));
	uint8_t lockState = (uint8_t)ui.bottomInfo.labelInfoGrp[5].sample->recvDate;
	if(index == 0)//如果处于采样界面
	{
		sample_update(lockState);
	}else{
		for (int i = 0; i < ARRAY_SIZE(_settingUI.labelGrp); i++)
		{
			viewGrp_Update(&(_settingUI.labelGrp[i]), i);
		}
	}
	//前提是开启自动锁
	if(_settingUI.iSwitchs & (1 << BIT_AUTO_LOCK))
	{
		//未锁定则一直查看锁定状态，若锁定则更新状态
		if(!lockState)
		{
			ui.bottomInfo.bFirstEntryLock = false;
			if(_settingUI._mods_sp[Type_Lock]->_attr._initVal == Lock_Proc_On)
			{
				_settingUI._mods_sp[Type_Lock]->_attr._initVal = Lock_Proc_handling;
				lv_anim_set_values(&_settingUI._mods_sp[Type_Lock]->anim, RANGE_MAX, RANGE_MAX / 2);
				lv_anim_start(&_settingUI._mods_sp[Type_Lock]->anim);
			}		
		}else{
			if(false == ui.bottomInfo.bFirstEntryLock)
			{	
				//进入锁定状态，更新所有配置数据
				ui.bottomInfo.bFirstEntryLock = true;
				Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, MAX_CONFIG_NUM * 2, E_Modbus_Read, 0);
			    _settingUI._mods_sp[Type_Lock]->_attr._initVal = Lock_Proc_On;
				lv_anim_start(&_settingUI._mods_sp[Type_Lock]->anim);
				return;
			}
		}
	}
	Gui_SendMessge(uart_mq, 0, MAX_SAMPLE_NUM * 2, E_Modbus_Read, 0);
}


void SubInfoGrp_Create(lv_obj_t* parent, SubInfo_t* info, const char* unitText)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 150, 39);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_AROUND,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    lv_obj_t* label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &font_bahnschrift_17, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    info->lableValue = label;

    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &font_bahnschrift_13, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xb3b3b3), 0);
    lv_label_set_text(label, unitText);
    info->lableUnit = label;
    info->cont = cont;
}



void BottomInfo_Create(lv_obj_t* parent)
{
	lv_obj_t* cont = lv_obj_create(parent);
	lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 90);
	lv_obj_align_to(cont, ui.topInfo.cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

	lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_flex_align(
		cont,
		LV_FLEX_ALIGN_SPACE_EVENLY,
		LV_FLEX_ALIGN_CENTER,
		LV_FLEX_ALIGN_CENTER
	);
	ui.bottomInfo.cont = cont;
	const char* unitText[OTHER_SAMPLE_NUMS] =
	{
		"Addr",						//地址
		"Integral",					//积分
		"Fast/slow",				//快/慢反
		"SA Signal",				//荧光
		"Error Signal",				//误差信号
		"Lock State",				//锁定状态
	};
	for (int i = 0; i < ARRAY_SIZE(ui.bottomInfo.labelInfoGrp); i++)
	{
		SubInfoGrp_Create(
			cont,
			&(ui.bottomInfo.labelInfoGrp[i]),
			unitText[i]
		);
		ui.bottomInfo.labelInfoGrp[i].sample = &ui_sample[i];		
	}
	if(ui_sample[LOCK_STATE_ADDR].recvDate == 0)
		ui.bottomInfo.bFirstEntryLock = false;
	else
		ui.bottomInfo.bFirstEntryLock = true;
}

/*********************************************************************************/
static void chart_draw_event_cb(lv_event_t* event)
{
    lv_obj_t* obj = lv_event_get_target(event);

    /*在线条绘制之前添加褪色的区域*/
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(event);
    if (dsc->part == LV_PART_ITEMS)
    {
        if (!dsc->p1 || !dsc->p2)
        {
            return;
        }
        /*添加一个线条蒙版，保持线条下方的区域*/
        lv_draw_mask_line_param_t line_mask_param;
        lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y, LV_DRAW_MASK_LINE_SIDE_BOTTOM);
        int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

        /*添加淡出效果:透明的底部覆盖顶部*/
        lv_coord_t h = lv_obj_get_height(obj);
        lv_draw_mask_fade_param_t fade_mask_param;
        lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP, obj->coords.y2);
        int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

        /*绘制一个受蒙版影响的矩形*/
        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_opa = LV_OPA_20;
        draw_rect_dsc.bg_color = dsc->line_dsc->color;

        lv_area_t area;
        area.x1 = dsc->p1->x;
        area.x2 = dsc->p2->x - 1;
        area.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
        area.y2 = obj->coords.y2;
        lv_draw_rect(dsc->draw_ctx, dsc->rect_dsc, &area);

        /*删除蒙版*/
        lv_draw_mask_free_param(&line_mask_param);
        lv_draw_mask_free_param(&fade_mask_param);
        lv_draw_mask_remove_id(line_mask_id);
        lv_draw_mask_remove_id(fade_mask_id);
    }
}



static void module_style_init(module_t* module, uint8_t icon_index, lv_coord_t w, lv_coord_t h)
{
    lv_obj_remove_style_all(module->obj_root);
    lv_obj_set_style_radius(module->obj_root, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(module->obj_root, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_bg_color(module->obj_root, lv_color_hex(0x393d45), LV_PART_MAIN);
    lv_obj_set_size(module->obj_root, w, h);
    lv_obj_clear_flag(module->obj_root, LV_OBJ_FLAG_SCROLLABLE);

    /*Icon*/
	lv_obj_remove_style_all(module->iconlabel);
	lv_obj_set_style_text_color(module->iconlabel, lv_color_hex(0xff931e), LV_PART_MAIN);
#if defined(USING_SECOND_TEMPER)
	lv_obj_set_style_text_font(module->iconlabel, &font_symbol_25, LV_PART_MAIN);
#else	
	lv_obj_set_style_text_font(module->iconlabel, &font_symbol_32, LV_PART_MAIN);
#endif
	lv_label_set_text(module->iconlabel, list2_icons[icon_index]);
	

	lv_obj_t *icon_label = lv_label_create(module->obj_root);
	lv_obj_remove_style_all(icon_label);
	lv_obj_set_style_text_color(icon_label, lv_color_hex(0xff931e), LV_PART_MAIN);
	lv_obj_set_style_text_font(icon_label, &font_tw_15, LV_PART_MAIN);
	lv_label_set_text(icon_label, list1_label[icon_index]);
#if defined(USING_SECOND_TEMPER)
	lv_obj_align(module->iconlabel, LV_ALIGN_TOP_LEFT, 6, 10);
	lv_obj_align_to(icon_label, module->iconlabel, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
#else	
	lv_obj_align(module->iconlabel, LV_ALIGN_TOP_LEFT, 15, 10);
	lv_obj_align_to(icon_label, module->iconlabel, LV_ALIGN_OUT_RIGHT_MID, 30, -8);
#endif   
    /*Utilize Label*/
    lv_obj_remove_style_all(module->usage_label);
    lv_obj_set_style_text_color(module->usage_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(module->usage_label, &font_bahnschrift_35, LV_PART_MAIN);
    lv_obj_align(module->usage_label, LV_ALIGN_CENTER, 0, 0);

    /*Utilize Chart*/
    lv_obj_move_background(module->usage_chart);
    lv_obj_set_style_bg_opa(module->usage_chart, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_line_width(module->usage_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(module->usage_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(module->usage_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_size(module->usage_chart, 0, LV_PART_INDICATOR);     //在折线上不显示点
    lv_obj_set_style_opa(module->usage_chart, LV_OPA_80, LV_PART_ITEMS);
    lv_obj_set_size(module->usage_chart, w, LV_PCT(80));
    lv_obj_align(module->usage_chart, LV_ALIGN_BOTTOM_LEFT, 0, -5);
    lv_chart_set_type(module->usage_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(module->usage_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(module->usage_chart, 30);
    lv_chart_set_update_mode(module->usage_chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(module->usage_chart, 10, 10);
    lv_obj_add_event_cb(module->usage_chart, chart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
}


static module_t* module_create(lv_obj_t* parent, uint8_t icon_index, lv_coord_t w, lv_coord_t h)
{
    module_t* _module = (module_t*)rt_malloc(sizeof(module_t));
    if (_module != NULL)
    {
        _module->obj_root = lv_obj_create(parent);
        _module->iconlabel = lv_label_create(_module->obj_root);
        _module->usage_label = lv_label_create(_module->obj_root);
        _module->usage_chart = lv_chart_create(_module->obj_root);
        _module->usage_series = lv_chart_add_series(_module->usage_chart, lv_color_hex(0x6a98fa)/*线条颜色*/, LV_CHART_AXIS_PRIMARY_Y);
		module_style_init(_module, icon_index, w, h);
		//采样值初始化
		_module->mod_sample = &ui_sample[DATA_OFFSET + icon_index];
    }
    return _module;
}

static void module_free(void)
{
	rt_free(ui.topInfo.I_module);
	rt_free(ui.topInfo.T1_module);	
#if defined(USING_SECOND_TEMPER)
	rt_free(ui.topInfo.T2_module);	
#endif	
}


void TopInfo_Create(lv_obj_t* parent)
{
    lv_obj_t* cont = lv_obj_create(parent);

    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, TOP_WITH, TOP_HEIGHT);
	lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );
	ui.topInfo.cont = cont;
#if defined(USING_SECOND_TEMPER)	
    ui.topInfo.I_module = module_create(cont, 0, SMALL_MODULE_WIDTH, MODULE_HEIGHT);
    ui.topInfo.T1_module = module_create(cont, 1, SMALL_MODULE_WIDTH, MODULE_HEIGHT);
    ui.topInfo.T2_module = module_create(cont, 2, SMALL_MODULE_WIDTH, MODULE_HEIGHT);
#else
    ui.topInfo.I_module = module_create(cont, 0, MODULE_WIDTH, MODULE_HEIGHT);	
    ui.topInfo.T1_module = module_create(cont, 1, MODULE_WIDTH, MODULE_HEIGHT);
#endif	
}


lv_obj_t* Btn_Create(lv_obj_t* parent, const void* img_src, lv_coord_t x_ofs)
{
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 40, 31);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, 0);
	/***********************************************************
	* 创建按键图标
	***********************************************************/
	lv_obj_t *label = lv_label_create(obj);
	lv_obj_set_style_text_font(label, &font_symbol_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
	lv_label_set_text(label, img_src);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

	/***********************************************************
	* 添加按钮按下或聚焦时的大小和颜色: 按下时变白, 松开后变黄
	***********************************************************/
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_width(obj, 45, LV_STATE_PRESSED);
    lv_obj_set_style_height(obj, 25, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x6a98fa), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xbbbbbb), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff931e), LV_STATE_FOCUSED);
    lv_obj_set_style_radius(obj, 9, 0);
	/******************************************
	* 添加按钮再按下或聚焦是的动态过渡效果
	******************************************/
    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] = {LV_STYLE_WIDTH, LV_STYLE_HEIGHT, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(
        &tran,
        prop,        	//初始化过渡动画需要的过渡变化形式，数组形式表现，数组最后一位必须为 0
        lv_anim_path_ease_out,
        200,
        0,
        NULL
    );
    lv_obj_set_style_transition(obj, &tran, LV_STATE_PRESSED);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_FOCUSED);
    lv_obj_update_layout(obj);
    return obj;
}







static void btnSetting_event_cb(lv_event_t* event)
{
	/*切换回设置界面，根据锁定状态决定是否进行数据刷新*/
	if(event->code == LV_EVENT_CLICKED)
	{
		lv_obj_t *setting_tile = lv_obj_get_parent(_settingUI._tabCont);
		lv_obj_t *root = lv_obj_get_parent(setting_tile);
		
		lv_obj_set_tile(root, setting_tile, LV_ANIM_ON);
	}
}

static void btnHome_event_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{
		uint8_t id = PAGE_MAIN;
		lv_group_set_editing(lv_group_get_default(), false);
		lv_group_remove_all_objs(lv_group_get_default());
		ui_cfgVal[0].recvDate = _settingUI.iSwitchs;
		anim_reback = 0;
		Gui_PageCallback((void *)&id, 1);	
	}
}


static void btnSave_event_cb(lv_event_t* event)
{
	if(event->code == LV_EVENT_CLICKED)
	{
		if(ui_Dialog.cont != NULL)
		{
			lv_label_set_text(ui_Dialog.title, "是否保存参数?");
			Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_Save);
		}
	}			
}



void BtnCont_Create(lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 40);
    lv_obj_align_to(cont, ui.bottomInfo.cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    ui.btnCont.cont = cont;
    ui.btnCont.btnSave = Btn_Create(cont, MY_ICON_SAVE, -80);
    lv_obj_add_event_cb(ui.btnCont.btnSave, btnSave_event_cb, LV_EVENT_CLICKED, NULL);
    ui.btnCont.btnHome = Btn_Create(cont, MY_ICON_HOME, 0);
	lv_obj_add_event_cb(ui.btnCont.btnHome, btnHome_event_cb, LV_EVENT_CLICKED, NULL);
    ui.btnCont.btnSetting = Btn_Create(cont, MY_ICON_ENRTY_SETTING, 80);
	lv_obj_add_event_cb(ui.btnCont.btnSetting, btnSetting_event_cb, LV_EVENT_CLICKED, NULL);
	sample_update(ui_sample[LOCK_STATE_ADDR].recvDate);
	//modbus每1秒采集一次
	ui.collect_timer = lv_timer_create(Info_TaskUpdate, 1000, (void *)par);
	lv_timer_resume(ui.collect_timer);	
}


void sample_tile_init(lv_obj_t* root)
{
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

	TopInfo_Create(root);
	BottomInfo_Create(root);
	BtnCont_Create(root);
	Gui_dialog_Create();
}




void sample_tile_exit(void)
{
	if(ui.collect_timer != NULL){
		lv_timer_pause(ui.collect_timer);
		lv_timer_del(ui.collect_timer);
	}
	module_free();
	lv_obj_del(ui_Dialog.cont);
	ui_Dialog.cont = NULL;
}


