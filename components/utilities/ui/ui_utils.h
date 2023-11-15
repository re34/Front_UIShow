#ifndef __UI_UTILS_H
#define __UI_UTILS_H

#include <rtthread.h>
#include <lvgl.h>

#if defined(RT_USING_USER_TRANSPORT)
	#include "mod_trans.h"
#endif

#define TIME_ON_TRIGGER		10

/**
 * @alarm_on                f0f3
 * @alarm_off               f1f6
 * @warn                    f071
 * @sound_on                f028
 * @sound_off               f6a9
 * @circle               	f111
 * @left               		f104
 * @right               	f105
 * @home               		f015
 * @return                	f14d


 * @temper               	f769
 * @bell					f0f3
 * @freq               		f4f0
 * @power_off               f011
 * @summary                 f201
 * @edit              		f0ad
 * @calendar                f073
 * @setting                 f1de
 * @record                  f044
 * @save                    f0c7
 * @lock                    f023
 * @unlock                  f13e
 * @history                 f56c
 * @login                   f234
 * @login_ok                f4fc
*/


//symbol_20(iconfont)
#define MY_ICON_ENRTY_SETTING 	"\xee\x98\x8d"
#define MY_ICON_TEMPER 			"\xee\x9c\x99"
#define MY_ICON_DDS 			"\xee\x99\xba"
#define MY_ICON_SWITCH 			"\xee\x98\xbb"
#define MY_ICON_CURRENT 		"\xee\xa2\xbd"
#define MY_ICON_MONITOR 		"\xee\x99\x87"
#define MY_ICON_LOCK 			"\xee\x98\xbd"
#define MY_ICON_UNLOCK 			"\xee\x98\xbf"
#define MY_ICON_POWER			"\xee\x98\xa6"
#define MY_ICON_SAVE			"\xee\x98\x81"
#define MY_ICON_HOME			"\xee\x99\x96" 
#define MY_ICON_MANUAL_UNLOCK	"\xee\xa0\xbc"
#define MY_ICON_AUTO_PEAK		"\xee\x9c\x98"
#define MY_ICON_AMPLIFY			"\xee\x98\x99"
#define MY_ICON_DEVELOP			"\xee\x98\x9a"
#define MY_ICON_LOGIN2			"\xee\x98\xb6"
#define MY_ICON_USER_OK			"\xee\x9f\x8a"


//symbol_32
#define MY_SYMBOL_SUMMARY 	"\xef\x88\x81"
#define MY_SYMBOL_SETTING 	"\xef\x87\x9e"
#define MY_SYMBOL_HISTORY 	"\xef\x95\xac"
#define MY_SYMBOL_TEMPER 	"\xef\x9d\xa9"
#define MY_SYMBOL_FREQ 		"\xef\x93\xb0"
#define MY_SYMBOL_EXIT	 	"\xef\x8b\xb6"
#define MY_SYMBOL_SAVE	 	"\xef\x83\x87"


#define MY_DEVICE_NAME		"Faraday Laser"
#define LOGIN_USER			"admin"			//用户名
#define LOGIN_PASSWORD		"12345678"		//密码

#define INIT_PAGE_ID		PAGE_BOOT




#define PAGE_INIT_DEF(TAG)  Gui_##TAG##Init
#define PAGE_FOCUS_DEF(TAG) Gui_##TAG##OnFocus
#define PAGE_EXIT_DEF(TAG) Gui_##TAG##Exit

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))




//LD参数数量
#define DEVELOP_ITEM_NUMS_END 			7     //常用参数数量
#define PARAM_ITEM_NUMS_END				15	  //其他参数数量
#define RELAY_NUMS 						8	  //开关继电器数量

#define DATA_OFFSET						6	  //采集(副)与采集（主）数组偏移
#define ALARM_OFFSET					7	  //
#define IDLE_OVERFLOW_TIME 				4	  //4X50 = 200ms空闲发送数据

#define RANGE_MIN	45			  //弧形进度条初始角
#define RANGE_MAX	315			  //弧形进度条终止角


#define PZT_AMP_MAX						14000 // 14V


enum {
	ENUM_TYPE_SW = 0,
	ENUM_TYPE_TXT,
};

enum PAGE_ID
{
	PAGE_NULL = -1,
	PAGE_PARAM = 0,
	PAGE_SUMMARY = 1,
	PAGE_SETTING = 2,
	PAGE_BOOT,
	PAGE_MAIN,
};

enum E_LockProc
{
	Lock_Proc_Off 		= 0,
	Lock_Proc_On		= 1,		
	Lock_Proc_handling 	= 2,
};

enum E_PowerProc
{
	Power_Proc_Off 		= 0,
	Power_Proc_On		= 1,		
};

enum E_ArcBarType
{
	Type_Power		= 0,
	Type_Lock		= 1,		
};

enum E_DialogType
{
	Dialog_Type_Save 			= 0,
	Dialog_Type_LockManual		= 1,
	Dialog_Type_LockAuto		= 2,
	Dialog_Type_LockOpenLaser   = 3,
	Dialog_Type_Power 			= 4,
	Dialog_Type_Tips 			= 5,
	Dialog_Type_Tips_Move 		= 6,	
	Dialog_Type_Success			= 7,
	Dialog_Type_Fail			= 8,
	Dialog_Type_TaSave			= 9,	
	Dialog_Type_None			= 10,	
};

enum E_LD_ItemIndex
{
	Item_Current   = 0,
	Item_Temper	   = 1,
	Item_PztAmp	   = 3,
	Item_PztBias   = 4,
};

//TA采样数据序列
enum E_TA_ItemIndex
{
	Item_Ta_Sample_I  	 = 0,
	Item_Ta_Setting_I  	 = 1,		
	Item_Ta_Sample_T	 = 2,
	Item_Ta_Setting_T    = 3,
};

enum E_TA_WarnIndex
{
	Item_Ta_Warn_Imax  	 = 2,
	Item_Ta_Warn_Tmin  	 = 5,		
	Item_Ta_Warn_Tmax	 = 6,

};

enum E_User_Authority
{
	Authority_User      	= 0,
	Authority_Developer	   = 1,
};

typedef void(*page_init_handle)(lv_obj_t* root);
typedef void(*page_focus_handle)(lv_obj_t* root);
typedef void(*page_exit_handle)(lv_obj_t* root);

typedef struct _PageObject
{
	uint8_t page_id;
	lv_obj_t* root;
	page_init_handle init_handler;
	page_focus_handle focus_handler;
	page_exit_handle exit_handler;
}M_PageObject;



#define ANIM_LINE_ADD(a, exec_cb, path_cb, ready_cb, time, back_time, delay, var, begin, end, repeatcnt) do{\
	lv_anim_init(a);\
	lv_anim_set_exec_cb(a, exec_cb);\
	if(path_cb != NULL) lv_anim_set_path_cb(a, path_cb);\
	if(ready_cb != NULL) lv_anim_set_ready_cb(a, ready_cb);\
	lv_anim_set_time(a, time);\
	lv_anim_set_playback_time(a, back_time);\
	lv_anim_set_delay(a, delay);\
	lv_anim_set_var(a, var);\
	lv_anim_set_values(a, begin, end);\
	lv_anim_set_repeat_count(a, repeatcnt);\
}while(0);



typedef struct {
    uint32_t time_base;
    uint32_t timeOut;
}lv_timeCount;


typedef struct {
	uint8_t inx;
	uint8_t objId;
} ui_locate_t;







typedef struct _dialog_t
{
	lv_obj_t* cont;
	lv_obj_t* title;
	lv_obj_t* btnOK;
	uint8_t bCheckFlag;
} dialog_t;	


struct m_attr_t
{
	bool bHasDot;
	uint8_t itemIndex;
	int32_t range_max;
	int32_t range_min;
	int32_t _initVal;
};

typedef struct _tab_module
{
	struct m_attr_t _attr;
    lv_obj_t* _root;
    lv_obj_t* _mObj;
}tab_module_t;

typedef struct _sw_module
{
	struct m_attr_t _attr;
    lv_obj_t *_mObj;
	lv_anim_t anim;
}sw_module_t;


typedef struct _enc_flewChk
{
	bool bIsTimerRun;
	bool bIsDataSend;
	bool bIsIncEnabled;
	uint32_t idleCnt;
	uint8_t targetInx;
	lv_timer_t *enc_flewTimer;
}enc_flewChk_t;



#if defined(UI_USING_PAGE_SETTING)

/********************************************************
* 设置界面
*******************************************************/


typedef struct
{
	lv_obj_t* cont;
	lv_obj_t* Bar_Main;
	lv_obj_t* Bar_icon;
}viewInfo_t;

struct _ui_Setting
{
	bool bIsAdmin;
    lv_obj_t* _tabCont;
	struct _tab_module  *_mods[PARAM_ITEM_NUMS_END];
	struct _tab_module  *_mods_sw[RELAY_NUMS];
	struct _sw_module  	*_mods_sp[2];
	viewInfo_t labelGrp[3];
	uint32_t iSwitchs;
};


/********************************************************
*  采样界面
*******************************************************/
typedef struct _module_t
{
    lv_obj_t* obj_root;
    lv_obj_t* iconlabel;
    lv_obj_t* usage_label;
    lv_obj_t* usage_chart;
    lv_chart_series_t* usage_series;
	Modbus_Date *mod_sample;	
}module_t;

typedef struct
{
	lv_obj_t* cont;
	lv_obj_t* lableValue;
	lv_obj_t* lableUnit;
	Modbus_Date *sample;
}SubInfo_t;


struct _ui_info
{
	struct
	{
		lv_obj_t* cont;
		module_t* I_module;
		module_t* T_module;
	}topInfo;
	struct
	{
		lv_obj_t* cont;
		SubInfo_t labelInfoGrp[6];
		bool bFirstEntryLock;
	} bottomInfo;
	struct
	{
		lv_obj_t* cont;
		lv_obj_t* btnSave;
		lv_obj_t* btnHome;
		lv_obj_t* btnSetting;
	} btnCont;
	struct
	{
		lv_obj_t* cont;
		lv_obj_t* title;
		lv_obj_t* btnOK;
	} errInfoCont;	
	lv_timer_t *collect_timer;
};
#endif
/*****************************TA界面************************************/


#if defined(UI_USING_PAGE_PARAM)
//HVPZT 相关参数

#define HVPZT_NUMS_END				3	  //高压pzt扫描参数
#define Scan_tileview_Page_Num 		2

enum E_IniLocal
{
	IniLocal_Inx_bias = 6,
	IniLocal_Inx_amp = 7,
	IniLocal_Inx_freq = 8,
};

struct _Ta_Setting
{
    lv_obj_t* _taCont;
	struct _tab_module  *_mods[HVPZT_NUMS_END];
	struct _sw_module  	*_mods_sp[1];
};
#endif



/******************************登录按钮****************************************/

#if defined(UI_USING_FUNC_LOGIN)

struct _login_info
{
	lv_obj_t* cont;
	lv_obj_t* kb;
	lv_obj_t* userInput;
	lv_obj_t* pwdInput;
	lv_obj_t* loginBtn;
};
#endif


extern uint8_t anim_reback;
extern struct _ui_Setting _settingUI;
extern struct _Ta_Setting _taUI;

extern struct _ui_info ui;
extern struct _login_info login_win;
extern struct _dialog_t ui_Dialog;

extern lv_style_t style_Dialog;
extern lv_style_t style_Info;
extern lv_style_t style_tabIcon;
extern lv_style_t style_ManualBtn;
extern lv_style_t style_tabContent;
extern lv_style_t style_Window;


//开机logo字体
LV_FONT_DECLARE(font_tw_15)
//采集显示界面字体
LV_FONT_DECLARE(font_bahnschrift_17)
LV_FONT_DECLARE(font_bahnschrift_13)
LV_FONT_DECLARE(font_bahnschrift_35)
LV_FONT_DECLARE(font_Atlantico_22)
//设置界面字体
LV_FONT_DECLARE(font_symbol_20)
//主界面字体
LV_FONT_DECLARE(font_symbol_32)
//汉字
LV_FONT_DECLARE(font_ch_16)




extern void Gui_bootInit(lv_obj_t* root);
extern void Gui_bootOnFocus(lv_obj_t* root);
extern void Gui_bootExit(lv_obj_t* root);

extern void Gui_mainInit(lv_obj_t* root);
extern void Gui_mainOnFocus(lv_obj_t* root);
extern void Gui_mainExit(lv_obj_t* root);

#if defined(UI_USING_PAGE_SETTING)
extern void Gui_settingInit(lv_obj_t* root);
extern void Gui_settingOnFocus(lv_obj_t* root);
extern void Gui_settingExit(lv_obj_t* root);

extern void sample_tile_init(lv_obj_t* root);
extern void sample_tile_exit(void);
extern void sw_flush_val(lv_obj_t *obj, uint8_t index, uint32_t val);

#endif

#if defined(UI_USING_PAGE_PARAM)
//TA setting
extern void Gui_paramInit(lv_obj_t* root);
extern void Gui_paramOnFocus(lv_obj_t* root);
extern void Gui_paramExit(lv_obj_t* root);
#endif


extern void Gui_loginInit(void);
extern void Gui_loginExit(void);





extern uint32_t Gui_Basic_GetOffset(uint32_t num, int32_t max, int32_t offset);
extern void Gui_AddToIndevGroup(lv_obj_t *obj);
extern void Gui_remove_IndevGroup(void);
extern void Gui_SendMessge(rt_mq_t mq, uint8_t addr, uint8_t regNum, uint8_t msgType, uint32_t val);

extern void Gui_DialogShow(struct _dialog_t *uiObj, lv_obj_t *obj, uint8_t type);
extern void Gui_setHightLight(lv_obj_t *obj, bool enable);
extern void Gui_setOutlineLight(lv_obj_t *obj, lv_color_t color, bool enable);

extern void spinbox_overFlow_send(bool enable);
extern void spinbox_flush_val(lv_obj_t *obj, uint32_t val);



extern bool Gui_CheckTimeOut(lv_timeCount *tm);
extern bool Gui_SetTimeOut(lv_timeCount *tm, uint32_t ms);
extern void Gui_PageCallback(void* arg, bool del);



extern void Gui_dialog_Create(void);
extern void Gui_menuInit(void);

extern void spinContent_style_init(tab_module_t* t_objBox, const char **label_list, lv_event_cb_t event_cb);
extern lv_obj_t *spinBtn_style_init(tab_module_t* t_objBox, lv_event_cb_t event_cb, void * user_data);


#endif

