#include "ui_utils.h"


struct _login_info login_win;
struct _dialog_t login_Dialog;


static void Msgbox_close_event_cb(lv_event_t * e)
{
	lv_obj_t *login_cont = (lv_obj_t *)lv_event_get_user_data(e);
	lv_obj_add_flag(login_Dialog.cont, LV_OBJ_FLAG_HIDDEN);
	if(Dialog_Type_Success  == login_Dialog.bCheckFlag)
	{
		lv_obj_add_flag(login_cont, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);	// 清除标志
		lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);	// 设置透明度
		//变更成登录成功图标
		lv_label_set_text(lv_obj_get_child(login_win.loginBtn, 0), MY_ICON_USER_OK);		
	}
}


//消息框
void Gui_MsgboxCreate(lv_obj_t *winObj)
{
    lv_obj_t *cont = lv_obj_create(lv_layer_top());
    lv_obj_set_size(cont, 230, 120);
    lv_obj_set_style_radius(cont, LV_PCT(5), LV_PART_MAIN);
	lv_obj_set_style_shadow_color(cont, lv_color_hex(0x7d71c3), LV_PART_MAIN);
	Gui_setHightLight(cont, true);
	lv_obj_align(cont, LV_ALIGN_CENTER, 0, 18);
	lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);    //默认隐藏
	login_Dialog.cont = cont;

    lv_obj_t * title = lv_label_create(cont);
	lv_obj_set_style_text_font(title, &font_ch_16, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
	login_Dialog.bCheckFlag = Dialog_Type_Success;
	login_Dialog.title = title;

	lv_obj_t *close_btn = lv_btn_create(cont);
	lv_obj_align_to(close_btn, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_obj_add_event_cb(close_btn, Msgbox_close_event_cb, LV_EVENT_CLICKED, (void *)winObj);
	lv_obj_t * label = lv_label_create(close_btn);
	lv_label_set_text(label, LV_SYMBOL_OK);
	const lv_font_t * font = lv_obj_get_style_text_font(close_btn, LV_PART_MAIN);
	lv_coord_t close_btn_size = lv_font_get_line_height(font) + LV_DPX(20);
	lv_obj_set_size(close_btn, close_btn_size, close_btn_size);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	login_Dialog.btnOK = close_btn;
}


/****************************************************************
*  用户及密码输入, 弹出键盘
***************************************************************/
static void login_input_event_cb(lv_event_t *e)
{
	struct _login_info *win = (struct _login_info *)lv_event_get_user_data(e);
	lv_obj_t *txt = lv_event_get_target(e);
	if(e->code == LV_EVENT_CLICKED)
	{
		lv_keyboard_set_textarea(win->kb, txt);
		lv_keyboard_set_mode(win->kb, win->pwdInput == txt? LV_KEYBOARD_MODE_NUMBER : LV_KEYBOARD_MODE_TEXT_LOWER);
		lv_obj_clear_flag(win->kb, LV_OBJ_FLAG_HIDDEN);
		lv_obj_align_to(win->cont, win->kb, LV_ALIGN_OUT_TOP_MID, 0, 60);
  }
	else if(e->code == LV_EVENT_READY || e->code == LV_EVENT_CANCEL)
	{
    	int len = strlen(lv_textarea_get_text(txt));
    	if(len != 0)
    	{
			lv_obj_align(win->cont, LV_ALIGN_CENTER, 0, 0);
			lv_obj_add_flag(win->kb, LV_OBJ_FLAG_HIDDEN);
			lv_keyboard_set_textarea(win->kb, NULL);
    	}
    }
}

/****************************************************************
*  登录认证校验， 弹出相应提示框
***************************************************************/
static void loginCheck_event_cb(lv_event_t *e)
{
	lv_obj_t *login_cont = (lv_obj_t *)lv_event_get_user_data(e);
	lv_obj_t *user_ta = lv_obj_get_child(login_cont, 2);
	lv_obj_t *pwd_ta = lv_obj_get_child(login_cont, 3);
    if(e->code == LV_EVENT_CLICKED)
	{
		const char * username = lv_textarea_get_text(user_ta);
		const char * password = lv_textarea_get_text(pwd_ta);
		/*********************************************************
		*  登录认证 : 1.登录成功则隐藏登录框，弹出消息框提示成功
		*             2. 验证失败,弹出提示
		*********************************************************/
		if((strcmp(username, LOGIN_USER) == 0) && (strcmp(password, LOGIN_PASSWORD) == 0))
		{
			 login_Dialog.bCheckFlag = Dialog_Type_Success;
			 lv_label_set_text(login_Dialog.title, "登录成功!");
			 _settingUI.bIsAdmin = Authority_Developer;
		}else{
			login_Dialog.bCheckFlag = Dialog_Type_Fail;
			lv_label_set_text(login_Dialog.title, "登录失败!");
			 _settingUI.bIsAdmin = Authority_User;
		}
		//弹出提示框
		if(lv_obj_has_flag(login_Dialog.cont, LV_OBJ_FLAG_HIDDEN))
		{
			lv_obj_clear_flag(login_Dialog.cont, LV_OBJ_FLAG_HIDDEN);
		}
    }
}
/****************************************************************
*  取消登录
***************************************************************/
static void loginCancel_event_cb(lv_event_t *e)
{
	lv_obj_t *login_cont = (lv_obj_t *)lv_event_get_user_data(e);
    if(e->code == LV_EVENT_CLICKED)
	{
		lv_obj_add_flag(login_cont, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);	// 清除标志
		lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);	// 设置透明度		
    }
}

/**********************************************************
*  在TOP层弹出登录框
*********************************************************/
static void loginShow_event_cb(lv_event_t* e)
{
	if(_settingUI.bIsAdmin == Authority_Developer)
		return;
	if(e->code == LV_EVENT_CLICKED)
	{
		lv_obj_t *login_cont = (lv_obj_t *)lv_event_get_user_data(e);
		Gui_DialogShow(NULL, login_cont, Dialog_Type_None);
	}else if(e->code == LV_EVENT_VALUE_CHANGED){
		//快捷登录
		lv_label_set_text(login_Dialog.title, "登录成功!");		
		_settingUI.bIsAdmin = Authority_Developer;
		Gui_DialogShow(&login_Dialog, NULL, Dialog_Type_Success);
	}
}


void Gui_loginInit(void)
{
	/*********************************************
	* 0 .创建登录框
	*********************************************/
	lv_obj_t *cont =  lv_obj_create(lv_layer_top());
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN); //登录框默认隐藏
	lv_obj_set_size(cont, 300, 240);
	lv_obj_set_style_radius(cont, 30, LV_PART_MAIN);
	lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN_WRAP); //列对齐
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_AROUND,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );
	login_win.cont = cont;
	/*********************************************
	* 1 .登录logo
	*********************************************/
	lv_obj_t * header = lv_label_create(cont);
	lv_obj_set_style_text_font(header, &lv_font_montserrat_22, LV_PART_MAIN);
	lv_label_set_text(header, LV_SYMBOL_HOME);
	lv_obj_t * label_title = lv_label_create(cont);
	lv_obj_set_style_text_font(label_title, &font_ch_16, LV_PART_MAIN);
	lv_label_set_text(label_title, "用户登录");
	/*********************************************
	* 2 .添加键盘
	*********************************************/
	lv_obj_t *kb = lv_keyboard_create(lv_layer_top());
	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	login_win.kb = kb;
	/*********************************************
	* 3 .添加用户及密码输入框
	*********************************************/
	//文本框1:用户名
    lv_obj_t *user_ta  = lv_textarea_create(cont);
    lv_textarea_set_text(user_ta, "");
	lv_textarea_set_placeholder_text(user_ta, "UseName");		//文本框在没有输入时显示的内容
	lv_textarea_set_one_line(user_ta, true);					//单行模式，只允许显示1行
	lv_textarea_set_max_length(user_ta, 11);					//只允许输入最多11个字符
	lv_obj_add_event_cb(user_ta, login_input_event_cb, LV_EVENT_ALL, (void *)&login_win);
	login_win.userInput = user_ta;

	//文本框2:密码
	lv_obj_t *pwd_ta = lv_textarea_create(cont);
    lv_textarea_set_text(pwd_ta, "");
	lv_textarea_set_placeholder_text(pwd_ta, "Password");
	lv_textarea_set_password_mode(pwd_ta, true);
	lv_textarea_set_one_line(pwd_ta, true);
	lv_textarea_set_max_length(pwd_ta, 11);
	lv_obj_add_event_cb(pwd_ta, login_input_event_cb, LV_EVENT_ALL, (void *)&login_win);
	login_win.pwdInput = pwd_ta;
	/*********************************************
	* 4 .添加登录按键及退出登录按钮
	*********************************************/
	lv_obj_t *btn_cont = lv_obj_create(cont);
	lv_obj_remove_style_all(btn_cont);
	lv_obj_set_size(btn_cont, 180, 35);
	//登录按钮		
	lv_obj_t *loginCheck = lv_btn_create(btn_cont);
	lv_obj_set_size(loginCheck, 85, 30);
	lv_obj_add_event_cb(loginCheck, loginCheck_event_cb, LV_EVENT_CLICKED, (void *)cont);
	lv_obj_t * label3 = lv_label_create(loginCheck);
	lv_label_set_text(label3, "Login");
 	lv_obj_align(label3, LV_ALIGN_CENTER, 0, 0);
	//退出登录按钮
	lv_obj_t *loginCancel = lv_btn_create(btn_cont);
	lv_obj_set_size(loginCancel, 85, 30);
	lv_obj_add_event_cb(loginCancel, loginCancel_event_cb, LV_EVENT_CLICKED, (void *)cont);
	lv_obj_align_to(loginCancel, loginCheck, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	label3 = lv_label_create(loginCancel);
	lv_label_set_text(label3, "Exit");
 	lv_obj_align(label3, LV_ALIGN_CENTER, 0, 0);	
	/*********************************************
	* 5 .添加登录框弹窗按钮
	*********************************************/
	lv_obj_t *loginEntry = lv_obj_create(lv_scr_act());
	lv_obj_set_size(loginEntry, 50, 50);
    lv_obj_set_style_bg_color(loginEntry, lv_color_hex(0x7d71c3), LV_PART_MAIN);
    lv_obj_set_style_bg_color(loginEntry, lv_color_hex(0x157ff6), LV_STATE_PRESSED);
	lv_obj_set_style_outline_width(loginEntry, 3, LV_PART_MAIN);
	lv_obj_set_style_outline_color(loginEntry, lv_color_hex(0x7d71c3), LV_PART_MAIN);
	lv_obj_set_style_radius(loginEntry, 30, LV_PART_MAIN);
	lv_obj_set_pos(loginEntry, 10, 50);
	lv_obj_clear_flag(loginEntry, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(loginEntry, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(loginEntry, loginShow_event_cb, LV_EVENT_CLICKED, (void *)cont);
	//快捷键触发回调
	lv_obj_add_event_cb(loginEntry, loginShow_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	login_win.loginBtn = loginEntry;

	lv_obj_t *login_Icon = lv_label_create(loginEntry);
	lv_obj_set_style_text_color(login_Icon, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(login_Icon, &font_symbol_20, LV_PART_MAIN);
	if(_settingUI.bIsAdmin == Authority_Developer)
		lv_label_set_text(login_Icon, MY_ICON_USER_OK);	
	else
		lv_label_set_text(login_Icon, MY_ICON_LOGIN2);
	lv_obj_align(login_Icon, LV_ALIGN_CENTER, 0, 0);
	/*********************************************
	* 6 .添加登录消息框
	*********************************************/
	Gui_MsgboxCreate(cont);
}

 
void Gui_loginExit(void)
{
	lv_obj_del(login_win.loginBtn);	 //删除登录框按钮	
	lv_obj_del(login_win.cont);		 //删除登录框主体
	lv_obj_del(login_win.kb);		 //删除键盘
	lv_obj_del(login_Dialog.cont);   //删除登录提示框
}


