#include "lv_port_indev.h"
#include "utils_list.h"
#include "ui_utils.h"
#include "ui_bar.h"
#if defined(RT_USING_USER_TRANSPORT)
#include "mod_trans.h"
#endif
#if defined(RT_USING_USER_PARA)
	#include "mod_para.h"
#endif

static M_listObject *menu_list;
static lv_group_t *group;

lv_style_t style_Dialog;
lv_style_t style_Info;
lv_style_t style_tabIcon;
lv_style_t style_ManualBtn;
lv_style_t style_tabContent;
lv_style_t style_Window;

struct _dialog_t ui_Dialog;

static M_PageObject page_boot = {
	.page_id				= PAGE_BOOT,
	.init_handler			= PAGE_INIT_DEF(boot),
	.focus_handler			= PAGE_FOCUS_DEF(boot),
	.exit_handler			= PAGE_EXIT_DEF(boot)
};

static M_PageObject page_main = {
	.page_id				= PAGE_MAIN,
	.init_handler			= PAGE_INIT_DEF(main),
	.focus_handler			= PAGE_FOCUS_DEF(main),
	.exit_handler			= PAGE_EXIT_DEF(main)
};

#if defined(UI_USING_PAGE_SETTING)

static M_PageObject page_setting = {
	.page_id				= PAGE_SETTING,
	.init_handler			= PAGE_INIT_DEF(setting),
	.focus_handler			= PAGE_FOCUS_DEF(setting),
	.exit_handler			= PAGE_EXIT_DEF(setting)
};
#endif

#if defined(UI_USING_PAGE_PARAM)
static M_PageObject page_param = {
	.page_id				= PAGE_PARAM,
	.init_handler			= PAGE_INIT_DEF(param),
	.focus_handler			= PAGE_FOCUS_DEF(param),
	.exit_handler			= PAGE_EXIT_DEF(param)
};
#endif


void Gui_AddToIndevGroup(lv_obj_t *obj)
{
    lv_group_add_obj(group, obj);
}



void Gui_remove_IndevGroup(void)
{
    lv_group_remove_all_objs(group);
}

#if defined(RT_USING_USER_TRANSPORT)
void Gui_SendMessge(rt_mq_t mq, uint8_t addr, uint8_t regNum, uint8_t msgType, uint32_t val)
{
	M_UartMsgEvent m_Msg_;
	
	m_Msg_.i_msgType = msgType;
	m_Msg_.i_addr = addr;
	m_Msg_.i_regNum = regNum;
	m_Msg_.w_data.recvDate = val;
	rt_mq_send(mq, &(m_Msg_), sizeof(m_Msg_)); 
}
#endif

void Gui_DialogShow(struct _dialog_t *uiObj, lv_obj_t *obj, uint8_t type)
{	
	lv_obj_t *tmpObj;
	if(Dialog_Type_None == type)
	{
		tmpObj = obj;
	}else{
		tmpObj = uiObj->cont;
		uiObj->bCheckFlag = type;
	}
	//防止操作前已有其他弹窗
	if(lv_obj_has_flag(tmpObj, LV_OBJ_FLAG_HIDDEN))
	{
		lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
		lv_obj_clear_flag(tmpObj, LV_OBJ_FLAG_HIDDEN);
		lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);							// 设置对象透明度
		lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0); // 设置对象颜色
	}
}



void Gui_setHightLight(lv_obj_t *obj, bool enable)
{
    if (enable) {
		lv_obj_set_style_border_width(obj, 2, 0);
		lv_obj_set_style_border_color(obj, lv_color_hex(0x7d71c3), LV_PART_MAIN);
        lv_obj_set_style_shadow_width(obj, 10, 0);
        lv_obj_set_style_shadow_spread(obj, 2, 0);
    } else {
		lv_obj_set_style_border_width(obj, 1, 0);
		lv_obj_set_style_border_color(obj, lv_color_make(30, 48, 80), LV_PART_MAIN);
        lv_obj_set_style_shadow_width(obj, 0, 0);
        lv_obj_set_style_shadow_spread(obj, 0, 0);
    }
}


void Gui_setOutlineLight(lv_obj_t *obj, lv_color_t color, bool enable)
{
    if (enable) {
		lv_obj_set_style_outline_width(obj, 3, LV_PART_MAIN);
		lv_obj_set_style_outline_color(obj, color, LV_PART_MAIN);
    } else {
		lv_obj_set_style_outline_width(obj, 0, LV_PART_MAIN);
    }
}




uint32_t Gui_Basic_GetOffset(uint32_t num, int32_t max, int32_t offset)
{
    if (num >= max) {
        LV_LOG_USER("[ERROR] num should less than max");
        return num;
    }
    uint32_t i;
    if (offset >= 0) {
        i = (num + offset) % max;
    } else {
        offset = max + (offset % max);
        i = (num + offset) % max;
    }
    return i;
}


bool Gui_CheckTimeOut(lv_timeCount *tm)
{
    int32_t isTmOut;
    isTmOut = (lv_tick_get() - (tm->time_base + tm->timeOut));
    if ((isTmOut > 0) || (0 == tm->time_base)) {
        tm->time_base = lv_tick_get();
        return true;
    } else {
        return false;
    }
}

bool Gui_SetTimeOut(lv_timeCount *tm, uint32_t ms)
{
    tm->time_base = lv_tick_get();
    tm->timeOut = ms;
    return true;
}


/*********************************************
* 使用迭代器查找对应id的节点 ---check ok
*********************************************/
static M_listNode *Gui_PageFind(M_listObject *ptList, uint8_t id)
{
	M_PageObject *p_page = NULL;
	M_listNode *p_node = NULL;

	list_iterator_t *it = list_iterator_new(ptList, LIST_HEAD);
	while ((p_node = list_iterator_next(it))) {
		p_page = (M_PageObject *)p_node->data;
		if(id == p_page->page_id)
			break;
	}
	rt_free(it);
	return p_node;
}

/*********************************************
* 注册插入新页面 ---check ok
*********************************************/

static void Gui_PageInsert(M_listObject *ptList, M_PageObject *page)
{
	if(ptList == NULL)
		return;
	/***********************************************
	* 1. 查看是否已有相同节点
	********************************************/
	M_listNode* node = Gui_PageFind(ptList, page->page_id);
	if(node != NULL)
	{
		return;
	}
	/***********************************************
	* 2. 新建节点加入链表尾部
	********************************************/
	node = list_rpush(ptList, list_node_new((void *)page));
	if(node != NULL)
		LV_LOG_USER("page add new!\n");
}


/*********************************************
* 将已插入链表的页面节点按id 来初始化 ---check ok
*********************************************/
static M_PageObject* Gui_PageCreate(M_listObject *ptList, uint8_t id)
{
	M_PageObject* page = NULL;
	M_listNode* node = Gui_PageFind(ptList, id);
	if(node != NULL)
	{
		page = (M_PageObject *)node->data;
		if(PAGE_SETTING != id)
        	page->root = lv_obj_create(lv_scr_act());
		else
			page->root = lv_tileview_create(lv_scr_act());
		/***********************************
		* 私有函数初始化
		************************************/
		page->init_handler(page->root);
	}
	return page;
}

//菜单界面切换回调函数
void Gui_PageCallback(void* arg, bool del)
{
	static uint8_t last_id = INIT_PAGE_ID;
	uint8_t Curr_Id =*(uint8_t *)arg;
	M_PageObject *p_page;
	lv_obj_t *root;

	//删除被切换掉的界面
	p_page = Gui_PageFind(menu_list, last_id)->data;
	if(p_page == NULL)
		return;
	if(del && p_page->root != NULL)
	{
		lv_anim_del_all();
		if(p_page->exit_handler)
			p_page->exit_handler(p_page->root);
		lv_obj_del(p_page->root);
		p_page->root = NULL;
	}
	//刷新刚切换的界面,若没有就新建
	p_page = Gui_PageFind(menu_list, Curr_Id)->data;
	if(p_page == NULL)
		return;
	if(p_page->root == NULL)
	{
		p_page = Gui_PageCreate(menu_list, Curr_Id);
	}
	root = p_page->root;
	if(root == NULL)
		return;
	p_page->focus_handler(root);
	last_id = p_page->page_id;
}


void Gui_IndevInit(void)
{
	group = lv_group_create();
	lv_group_set_default(group);
#if defined(RT_USING_ENCODER_INPUTDEV)
	lv_indev_set_group(enc_indev_obj, group);
#endif
#if defined(RT_USING_KEYPAD_INPUTDEV)
	lv_indev_set_group(key_indev_obj, group);
#endif
	lv_indev_set_group(ts_indev_obj, group);
}


void Gui_Style_Init(void)
{
	//标签页图标风格
	lv_style_init(&style_tabIcon);
	lv_style_set_text_color(&style_tabIcon, lv_color_hex(0x29a6ac));
	lv_style_set_text_font(&style_tabIcon, &font_symbol_20);
	//手动类按钮风格
	lv_style_init(&style_ManualBtn);
	lv_style_set_obj_size(&style_ManualBtn, 75, 40);
	lv_style_set_border_width(&style_ManualBtn, 0);
	lv_style_set_bg_color(&style_ManualBtn, lv_color_hex(0x3b67b0));
	lv_style_set_radius(&style_ManualBtn, 20);
	//标签页内容布局
	lv_style_init(&style_tabContent);
    lv_style_set_pad_hor(&style_tabContent, 8);     //设置左右离外边框的间距
	lv_style_set_pad_ver(&style_tabContent, 10);	//设置上下离外边框的间距
	lv_style_set_pad_row(&style_tabContent, -10); 	//设置各item之间的行间距
	lv_style_set_pad_column(&style_tabContent, 20); //设置各item之间的列间距

	//小窗口风格
	lv_style_init(&style_Window);
	lv_style_set_radius(&style_Window, 15);						 	//圆角
	lv_style_set_bg_color(&style_Window, lv_color_hex(0x3b4066));   //框区域颜色
  	lv_style_set_border_width(&style_Window, 0);	   				//无边框
	//消息框风格
	lv_style_init(&style_Dialog);
	lv_style_set_obj_size(&style_Dialog, 280, 120);
	lv_style_set_bg_color(&style_Dialog, lv_color_hex(0x15294c));
	lv_style_set_radius(&style_Dialog, LV_PCT(5));
	lv_style_set_shadow_color(&style_Dialog, lv_color_hex(0x15294c));
	lv_style_set_shadow_width(&style_Dialog, 8);
	lv_style_set_shadow_spread(&style_Dialog, 2);
	lv_style_set_align(&style_Dialog, LV_ALIGN_CENTER);

	//进度条样式（蓝到红渐变）
	lv_style_init(&style_Info);
	lv_style_set_bg_opa(&style_Info, LV_OPA_COVER);
	lv_style_set_bg_color(&style_Info, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_bg_grad_color(&style_Info, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_bg_grad_dir(&style_Info, LV_GRAD_DIR_HOR);
}


void spinContent_style_init(tab_module_t* t_objBox, const char **label_list, lv_event_cb_t event_cb)
{
	//初始化为空白对象(无边框)
	char initStr[7];
	struct m_attr_t *p_attr = &t_objBox->_attr;
	//整体样式
	lv_obj_remove_style_all(t_objBox->_root);
	lv_obj_set_size(t_objBox->_root, 220, 85);

	//标签样式
  	lv_obj_t *label = lv_label_create(t_objBox->_root);
	lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
	lv_label_set_text(label, label_list[p_attr->itemIndex]);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
	//spinbox样式
    lv_spinbox_set_range(t_objBox->_mObj, p_attr->range_min, p_attr->range_max);
	if(p_attr->bHasDot){
		lv_spinbox_set_digit_format(t_objBox->_mObj, 6, 3);
		lv_snprintf(initStr, sizeof(initStr), "%.3f", ((float)p_attr->_initVal) / 1000);
	}else{
		if(p_attr->range_max <= 999)
			lv_spinbox_set_digit_format(t_objBox->_mObj, 3, 3);
		else
			lv_spinbox_set_digit_format(t_objBox->_mObj, 7, 7);
		lv_snprintf(initStr, sizeof(initStr), "%d", p_attr->_initVal);
	}	
	lv_spinbox_set_cursor_pos(t_objBox->_mObj, 0);
	lv_obj_set_size(t_objBox->_mObj, 130, 45);
	lv_textarea_set_align(t_objBox->_mObj, LV_TEXT_ALIGN_CENTER);
	lv_obj_set_style_outline_width(t_objBox->_mObj, 5, LV_STATE_EDITED);   	//选中框边框加粗	
	lv_obj_set_style_text_font(t_objBox->_mObj, &lv_font_montserrat_22,  LV_PART_MAIN);
	lv_obj_align(lv_textarea_get_label(t_objBox->_mObj), LV_ALIGN_TOP_MID, 0, 3);
  	lv_obj_align_to(t_objBox->_mObj, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
	//初始值赋值
	lv_textarea_set_text((lv_obj_t *)t_objBox->_mObj, initStr);
	lv_spinbox_set_value(t_objBox->_mObj, p_attr->_initVal);
	t_objBox->_mObj->user_data = (void *)p_attr;
    lv_obj_add_event_cb(t_objBox->_mObj, event_cb, LV_EVENT_ALL, NULL);
}

void spinbox_flush_val(lv_obj_t *obj, uint32_t val)
{
	char initStr[7];
	struct m_attr_t *p_attr = (struct m_attr_t *)obj->user_data;

	if(p_attr->bHasDot){
		lv_snprintf(initStr, sizeof(initStr), "%.3f", ((float)val) / 1000);
	}else{
		lv_snprintf(initStr, sizeof(initStr), "%d", val);
	}	
	lv_textarea_set_text(obj, initStr);
	lv_spinbox_set_value(obj, val);
}


lv_obj_t *spinBtn_style_init(tab_module_t* t_objBox, lv_event_cb_t event_cb, void * user_data)
{
	//左右按键设置
	lv_obj_t * btn = lv_btn_create(t_objBox->_root);
	lv_obj_set_size(btn, 30, 30);
	lv_obj_set_style_text_font(btn, &lv_font_montserrat_14,  LV_PART_MAIN);
	btn->user_data = (void *)&t_objBox->_attr.itemIndex;
	lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, user_data);
  	lv_group_remove_obj(btn); //默认不被聚焦
  	return btn;
}


static void btnOk_event_cb(lv_event_t* e)
{
    if(e->code == LV_EVENT_CLICKED)
    {
        lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));

		lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度
        lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
		switch(ui_Dialog.bCheckFlag)
		{
			//保存
			case Dialog_Type_Save:
				//Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, 2, E_Modbus_Write, _settingUI.iSwitchs |(1 << BIT_SAVE));
			break;
			case Dialog_Type_TaSave:
#if defined(RT_USING_USER_PARA)				
				for(int i = IniLocal_Inx_bias; i < IniLocal_Inx_bias + HVPZT_NUMS_END; i++ )
				{
					Set_SystemParam(i);
				}
#endif
			break;
			case Dialog_Type_Power:		
				lv_event_send(_taUI._mods_sp[Type_Power]->_mObj, LV_EVENT_VALUE_CHANGED, NULL);
			break;			
			//提示类对话框
			case Dialog_Type_Tips:
		        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);  
		        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  
		        lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);				
			break;			
#if defined(UI_USING_PAGE_SETTING)
			case Dialog_Type_Tips_Move:
		        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);  
		        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  
		        lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
				//回滚到默认页
				lv_tabview_set_act(_settingUI._tabCont, 0, LV_ANIM_ON);
			break;
			//单一开关机
			case Dialog_Type_Power:
				//关机操作需要先解锁
				if(_settingUI._mods_sp[Type_Power]->_attr._initVal == Power_Proc_On 
					&& _settingUI._mods_sp[Type_Lock]->_attr._initVal >= Lock_Proc_On)
					lv_event_send(_settingUI._mods_sp[Type_Lock]->_mObj, LV_EVENT_VALUE_CHANGED, NULL);				
				lv_event_send(_settingUI._mods_sp[Type_Power]->_mObj, LV_EVENT_VALUE_CHANGED, NULL);

			break;
			//一键锁定(开机 + 锁定)
			case Dialog_Type_LockOpenLaser: 
				lv_event_send(_settingUI._mods_sp[Type_Power]->_mObj, LV_EVENT_VALUE_CHANGED, NULL);
				lv_event_send(_settingUI._mods_sp[Type_Lock]->_mObj, LV_EVENT_VALUE_CHANGED, NULL);
			break;
			default:
			{
				//单一锁定：加前提条件，执行锁定时，需先开电源
				if(_settingUI._mods_sp[Type_Power]->_attr._initVal == Power_Proc_On)
					lv_event_send(_settingUI._mods_sp[Type_Lock]->_mObj, LV_EVENT_VALUE_CHANGED, NULL);
				else{
					//弹窗提示
					if(ui_Dialog.cont != NULL)
					{
						lv_label_set_text(ui_Dialog.title, "请先打开电源!");
						Gui_DialogShow(&ui_Dialog, NULL, Dialog_Type_Tips);
					}
				}
			}		
			break;
#endif
		}
    }
}


static void btnCancel_event_cb(lv_event_t* e)
{
    if(e->code == LV_EVENT_CLICKED)
    {
        lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);   // 清除标志
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);  // 设置透明度
        lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
#if defined(UI_USING_PAGE_SETTING)			
		if(ui_Dialog.bCheckFlag == Dialog_Type_Tips_Move)					
			lv_tabview_set_act(_settingUI._tabCont, 0, LV_ANIM_ON);//回滚到默认页
#endif
    }

}

void Gui_dialog_Create(void)
{
    lv_obj_t *cont = lv_obj_create(lv_layer_top());
	lv_obj_add_style(cont, &style_Dialog, LV_PART_MAIN);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN); //默认隐藏
    ui_Dialog.cont = cont;

    lv_obj_t * title = lv_label_create(cont);
	lv_obj_set_style_text_color(title, lv_color_hex(0xf3b560), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &font_ch_16, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
	ui_Dialog.title = title;
	
	lv_obj_t *okBtn = lv_btn_create(cont);
	lv_obj_set_size(okBtn, 45, 25);
	lv_obj_align(okBtn, LV_ALIGN_BOTTOM_MID, -45, -10);
	lv_obj_set_style_bg_img_src(okBtn, LV_SYMBOL_OK, 0);
	lv_obj_add_event_cb(okBtn, btnOk_event_cb, LV_EVENT_CLICKED, NULL);
	ui_Dialog.btnOK = okBtn;

	lv_obj_t *CancelBtn = lv_btn_create(cont);
	lv_obj_set_size(CancelBtn, 45, 25);
	lv_obj_align(CancelBtn, LV_ALIGN_BOTTOM_MID, 45, -10);
	lv_obj_set_style_bg_img_src(CancelBtn, LV_SYMBOL_CLOSE, LV_PART_MAIN);
    lv_obj_add_event_cb(CancelBtn, btnCancel_event_cb, LV_EVENT_CLICKED, NULL);
}



/*********************************************
* 菜单初始化 ---check ok
*********************************************/
void Gui_menuInit(void)
{
	menu_list = list_new();
	if(menu_list == NULL)
		return;
	//可用输入设备初始化
	Gui_IndevInit();
	//添加启动页面
	Gui_Style_Init();
	Gui_PageInsert(menu_list, &page_boot);
	Gui_PageInsert(menu_list, &page_main);
#if defined(UI_USING_PAGE_SETTING)
	Gui_PageInsert(menu_list, &page_setting);
#endif
#if defined(UI_USING_PAGE_PARAM)
	Gui_PageInsert(menu_list, &page_param);
#endif
	//LOGO页面创建
	M_PageObject *page = Gui_PageCreate(menu_list, PAGE_BOOT);
	if(page != NULL)
	{
		page->focus_handler(page->root);
	}
}


