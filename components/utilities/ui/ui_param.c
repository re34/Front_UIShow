#include "lvgl.h"
#include "lv_port_indev.h"
#include "ui_utils.h"
#include "spi_ad5541.h"
#if defined(RT_USING_USER_TRANSPORT)
	#include "mod_trans.h"
#endif
#if defined(RT_USING_USER_PARA)
	#include "mod_para.h"
#endif


#if defined(UI_USING_PAGE_PARAM)

#define Index_Scan_Button  	0


enum E_ScanItem
{
	ScanItem_bias		= 0,
	ScanItem_amp		= 1,	
	ScanItem_freq		= 2,
};

struct _Ta_Setting _taUI;

const char* Ta_list[HVPZT_NUMS_END] = {
	"扫描偏置(V)",	
	"扫描幅值(V)",
	"扫描频率(HZ)",	
};


static void param_inc_event_cb(lv_event_t * e)
{
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		Dac_Para *sPara =  (Dac_Para *)e->user_data;
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
        lv_spinbox_increment(_taUI._mods[index]->_mObj);	
		sPara->params[index] = lv_spinbox_get_value(_taUI._mods[index]->_mObj);	
		rt_kprintf("inc sPara->params[%d] = %ld\n", index, sPara->params[index]);
		if(sPara->chipVal.scanState > State_In_ScanClose)
			rt_sem_release(&param_Sem);		
    }
}

static void param_dec_event_cb(lv_event_t * e)
{
    if(e->code == LV_EVENT_SHORT_CLICKED) {
		Dac_Para *sPara =  (Dac_Para *)e->user_data;
		lv_obj_t *obj = lv_event_get_target(e);
		uint8_t index = *((uint8_t *)obj->user_data);
		lv_spinbox_decrement(_taUI._mods[index]->_mObj);
		sPara->params[index] = lv_spinbox_get_value(_taUI._mods[index]->_mObj);
		rt_kprintf("dec sPara->params[%d] = %ld\n", index, sPara->params[index]);
		if(sPara->chipVal.scanState > State_In_ScanClose)
			rt_sem_release(&param_Sem);
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
	//中间微动输入框
	spinContent_style_init(t_objBox, &Ta_list[0], parambox_event_cb);
	//左右加减控制键
	lv_obj_t * btn = spinBtn_style_init(t_objBox, param_inc_event_cb, (void *)&_g_ParasPool._uScanPara);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
	
	btn = spinBtn_style_init(t_objBox, param_dec_event_cb, (void *)&_g_ParasPool._uScanPara);
	lv_obj_align_to(btn, t_objBox->_mObj, LV_ALIGN_OUT_LEFT_MID, -8, 0);
	lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
}


tab_module_t *TA_SubCreate(lv_obj_t * parent, uint8_t inx)
{
    tab_module_t* t_tabBox = (tab_module_t *)rt_malloc(sizeof(tab_module_t));
    if (t_tabBox != NULL)
    {
		t_tabBox->_attr.itemIndex = inx;
		switch(inx)
		{
			case ScanItem_bias:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.range_min = 0;
				t_tabBox->_attr.range_max = 100000;
			break;
			case ScanItem_amp:
				t_tabBox->_attr.bHasDot = true;
				t_tabBox->_attr.range_min = 1000;
				t_tabBox->_attr.range_max = 50000;
			break;
			case ScanItem_freq:
				t_tabBox->_attr.bHasDot = false;
				t_tabBox->_attr.range_min = 1;
				t_tabBox->_attr.range_max = 50;
			break;			
		}
		t_tabBox->_attr._initVal = _g_ParasPool._uScanPara.params[inx];
        t_tabBox->_root = lv_obj_create(parent);	
		t_tabBox->_mObj = lv_spinbox_create(t_tabBox->_root);
		paramBox_style_init(t_tabBox);	
    }
    return t_tabBox;
}



static void tileview_load_event_cb(lv_event_t* e)
{
	if(e->code == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *tv_obj = lv_event_get_target(e);
		lv_obj_t *btn_cont = lv_event_get_user_data(e);	
		
		uint8_t index = lv_obj_get_index(lv_tileview_get_tile_act(tv_obj));
		Gui_setOutlineLight(lv_obj_get_child(btn_cont, index), lv_color_hex(0xff931e), true);
		for(int i = 0; i < Scan_tileview_Page_Num; i++)
		{
			if(i != index)
				Gui_setOutlineLight(lv_obj_get_child(btn_cont, i), lv_color_white(), false);			
		}
	}
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

/*******************************************************************************
*扫描按钮
*******************************************************************************/
static void anim_Scan_process(void* var, int32_t value)
{
	lv_arc_set_end_angle((lv_obj_t *)var, value); //设置弧形控件的结束角度
}

static void anim_Scan_finish(lv_anim_t *a)
{
	lv_obj_t *label_cont = lv_obj_get_child((lv_obj_t *)a->var, 0);
	lv_obj_t *label = lv_obj_get_child(label_cont, 0);
	struct m_attr_t *p_attr = (struct m_attr_t *)label_cont->user_data;
	Dac_Para *sPara =  (Dac_Para *)a->user_data;
	if(p_attr->_initVal == Power_Proc_Off)
	{
		//开启扫描(绿色)
		p_attr->_initVal = Power_Proc_On;
		sPara->chipVal.scanState = Power_Proc_On;
		lv_anim_set_values(&_taUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_max, p_attr->range_min);
		lv_obj_set_style_text_color(label, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
		rt_sem_release(&scanSw_Sem);
	}
	else{
		//关闭扫描(红色)
		p_attr->_initVal = Power_Proc_Off;
		sPara->chipVal.scanState = Power_Proc_Off;
		lv_anim_set_values(&_taUI._mods_sp[p_attr->itemIndex]->anim, p_attr->range_min, p_attr->range_max);
		lv_obj_set_style_text_color(label, lv_color_hex(0xd74047), LV_PART_MAIN);
		rt_sem_release(&param_Sem);
	}
}


static void ScanButton_event_cb(lv_event_t *e)
{
	lv_obj_t *label_cont = lv_event_get_target(e);
	struct m_attr_t *p_attr = (struct m_attr_t *)label_cont->user_data; 

	if(e->code == LV_EVENT_CLICKED)
	{
		if(ui_Dialog.cont != NULL)
		{
			if(p_attr->_initVal == Power_Proc_On)
				lv_label_set_text(ui_Dialog.title, "是否关闭扫描?");
			else
				lv_label_set_text(ui_Dialog.title, "是否打开扫描?");
			ui_Dialog.bCheckFlag = Dialog_Type_Power;			
			Gui_DialogShow(&ui_Dialog, NULL, ui_Dialog.bCheckFlag);
		}	
	}
	else if(e->code == LV_EVENT_VALUE_CHANGED)
	{	
		if(p_attr->itemIndex == Index_Scan_Button)//扫描键功能
		{	
			lv_anim_start(&_taUI._mods_sp[p_attr->itemIndex]->anim);			
		}
	}
}




sw_module_t *ui_ScanButtonCreate(lv_obj_t * root, uint8_t index, void *ext_data)
{
    sw_module_t* t_swMod = (sw_module_t *)rt_malloc(sizeof(sw_module_t));
	if(t_swMod)
	{
		struct m_attr_t *p_attr = &t_swMod->_attr;
		Dac_Para *sPara =  (Dac_Para *)ext_data;
		p_attr->range_min = RANGE_MIN;
		p_attr->range_max = RANGE_MAX;
		p_attr->itemIndex = index;
		/**********************************************************
		*1. 初始化圆环
		*********************************************************/
		lv_obj_t * arc_obj = lv_arc_create(root);
		lv_obj_set_size(arc_obj, 85, 85);
		lv_arc_set_bg_angles(arc_obj, p_attr->range_min, p_attr->range_max);
		lv_arc_set_rotation(arc_obj, 90);
		lv_obj_set_style_arc_color(arc_obj, lv_color_hex(0x53597d), LV_PART_MAIN);
		lv_obj_remove_style(arc_obj, NULL, LV_PART_KNOB);		     //移除按钮
		lv_obj_set_style_arc_width(arc_obj, 14, LV_PART_INDICATOR);  //设置背景圆弧粗细
		lv_obj_set_style_arc_width(arc_obj, 14, LV_PART_MAIN);       //设置前景圆弧粗细
		lv_obj_clear_flag(arc_obj, LV_OBJ_FLAG_CLICKABLE);		  	 //禁止按键		
		lv_obj_align(arc_obj, LV_ALIGN_CENTER, 0, 30);
		/**********************************************************
		*2. 初始化圆环核心按键
		*********************************************************/		
		lv_obj_t * arc_label_cont = lv_obj_create(arc_obj);
		lv_obj_set_size(arc_label_cont, 55, 55);
		lv_obj_set_style_bg_color(arc_label_cont, lv_color_hex(0x3b4066), LV_PART_MAIN);
		lv_obj_set_style_border_width(arc_label_cont, 0, LV_PART_MAIN);
		lv_obj_set_style_radius(arc_label_cont, 30, LV_PART_MAIN);
		lv_obj_align(arc_label_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_flag(arc_label_cont, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_clear_flag(arc_label_cont, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_event_cb(arc_label_cont, ScanButton_event_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_add_event_cb(arc_label_cont, ScanButton_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
		arc_label_cont->user_data = (void *)p_attr;
		t_swMod->_mObj = arc_label_cont;
		/**********************************************************
		*3. 初始化圆环核心图片
		*********************************************************/
		lv_obj_t * arc_label = lv_label_create(arc_label_cont);
		lv_obj_set_style_text_font(arc_label, &font_symbol_20, LV_PART_MAIN);
		lv_label_set_text(arc_label, MY_ICON_POWER);
		lv_obj_align(arc_label, LV_ALIGN_CENTER, 0, 0);

		lv_obj_t *label = lv_label_create(root);
		lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
		lv_label_set_text(label, "扫描开关");
		lv_obj_align_to(label, arc_obj, LV_ALIGN_OUT_TOP_MID, 0, -15);
		/**********************************************************
		*3. 初始化动画
		*********************************************************/
		lv_anim_init(&t_swMod->anim);													// 初始化动画
		lv_anim_set_var(&t_swMod->anim, arc_obj);										// 设置动画对象为arc_obj
		lv_anim_set_time(&t_swMod->anim, 1000); 										// 设置动画时间1秒
		lv_anim_set_user_data(&t_swMod->anim, ext_data);								// 设置私有数据
		lv_anim_set_exec_cb(&t_swMod->anim, (lv_anim_exec_xcb_t)anim_Scan_process);		//设置动画回调函数
		lv_anim_set_ready_cb(&t_swMod->anim, anim_Scan_finish);	

		/**********************************************************
		*3. 初始化值
		*********************************************************/		
		p_attr->_initVal = sPara->chipVal.scanState;
		if(p_attr->_initVal){
			lv_arc_set_angles(arc_obj, p_attr->range_min, p_attr->range_max);
			//已开启扫描(绿色)
			lv_obj_set_style_text_color(arc_label, lv_color_hex(0x4ab3b0), LV_PART_MAIN);
			lv_anim_set_values(&t_swMod->anim, p_attr->range_max, p_attr->range_min);
		}
		else{
			lv_arc_set_value(arc_obj, 0);
			//未开启扫描(红色)
			lv_obj_set_style_text_color(arc_label, lv_color_hex(0xd74047), LV_PART_MAIN);
			lv_anim_set_values(&t_swMod->anim, p_attr->range_min, p_attr->range_max);
		}
	}
	return t_swMod;
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
	* (一) 扫描开关按键区
	**************************************************************************/
	lv_obj_t *sw_root = lv_obj_create(root);
	lv_obj_set_size(sw_root, 150, 240);
	lv_obj_add_style(sw_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(sw_root, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_clear_flag(sw_root, LV_OBJ_FLAG_SCROLLABLE);
	_taUI._mods_sp[0] = ui_ScanButtonCreate(sw_root, 0, (void *)&_g_ParasPool._uScanPara);
	/*************************************************************************
	* (二) 用户设置区(滑动分页)
	**************************************************************************/
	lv_obj_t *user_root = lv_obj_create(root);
	lv_obj_set_size(user_root, 285, 240);
	lv_obj_add_style(user_root, &style_Window, LV_PART_MAIN);
	lv_obj_align(user_root, LV_ALIGN_RIGHT_MID, 0, 0);
	lv_obj_clear_flag(user_root, LV_OBJ_FLAG_SCROLLABLE);
	
	lv_obj_t *tvcont = lv_tileview_create(user_root);
	lv_obj_remove_style_all(tvcont);
	lv_obj_set_size(tvcont, 285, 190);
	lv_obj_align(tvcont, LV_ALIGN_TOP_MID, 0, 0);	
	lv_obj_set_scrollbar_mode(tvcont, LV_SCROLLBAR_MODE_OFF);
	/*************************************************************************
	* 1. 电流设置区(分页1)
	**************************************************************************/
    lv_obj_t* current_tile = lv_tileview_add_tile(tvcont, 0, 0, LV_DIR_RIGHT);
	lv_obj_set_flex_flow(current_tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(current_tile, 30, LV_PART_MAIN);
	lv_obj_set_style_pad_row(current_tile, 0, LV_PART_MAIN);
	for(;i < HVPZT_NUMS_END; i++)
	{
    	_taUI._mods[i] = TA_SubCreate(current_tile, i); 
	}
	/*************************************************************************
	* 2. 保存设置区(分页2)
	**************************************************************************/		
  	lv_obj_t* other_tile = lv_tileview_add_tile(tvcont, 1, 0, LV_DIR_LEFT);
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
	for(i = 0; i < Scan_tileview_Page_Num; i++)
	{
		tvBtn = lv_btn_create(tvBtn_cont);
		lv_obj_set_size(tvBtn, 15, 15);
		lv_obj_set_style_bg_color(tvBtn, lv_color_white(), LV_PART_MAIN);
		lv_obj_set_style_radius(tvBtn, 15, LV_PART_MAIN);
		Gui_setOutlineLight(tvBtn, lv_color_hex(0xff931e), i == 0?true:false);	
		if(i == 0)
			lv_obj_align(tvBtn, LV_ALIGN_CENTER, -13, 0);
		else
			lv_obj_align(tvBtn, LV_ALIGN_CENTER, 13, 0);			
	}
	//翻页回调
	lv_obj_add_event_cb(tvcont, tileview_load_event_cb, LV_EVENT_VALUE_CHANGED, (void *)tvBtn_cont);
	Gui_dialog_Create();
}



void Gui_paramOnFocus(lv_obj_t* root)
{
	return;
}

void Gui_paramExit(lv_obj_t* root)
{	
	int i;
	for( i = 0; i < HVPZT_NUMS_END; i++)
	{
		rt_free(_taUI._mods[i]);			
	}
	for(i = 0; i < 1; i++)
	{
		rt_free(_taUI._mods_sp[i]);						
	}	
	lv_obj_del(ui_Dialog.cont);
	ui_Dialog.cont = NULL;
	return;
}

#endif

