#ifndef __UI_UTILS_H
#define __UI_UTILS_H

#include <rtthread.h>
#include <lvgl.h>
#include "mod_trans.h"

#define TIME_ON_TRIGGER		10
#define IDLE_OVERFLOW_TIME 	4	  //4X50 = 200ms空闲发送数据
#define DEFAULT_TIME_CNT	120

#define ROLE_USER		0
#define ROLE_DEVELOPER	1

#define SW_ONESTEP		0
#define SW_AUTOLOCK		1


/*********************************************************
*					lvgl图标UTF码表
**********************************************************/
#define SUPPORT_ICONFONT_FONT

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

#if defined(SUPPORT_ICONFONT_FONT)
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
//新增 symbol_32
#define MY_SYMBOL_LIGHT_OPEN			"\xee\x98\x87"
#define MY_SYMBOL_LIGHT_CLOSE			"\xee\x98\xa7"
#define MY_SYMBOL_FIBER_ENABLE			"\xee\x98\xa2"
#define MY_SYMBOL_FIBER_DISABLE			"\xee\xa1\x9e"
#define MY_SYMBOL_WORK_MODE				"\xee\x98\xb4"

#define MY_SYMBOL_FIBER_CONFIG 		"\xef\x87\x81"
#define MY_SYMBOL_SETTING 			"\xee\x98\xa9"
#define MY_SYMBOL_LD_CONFIG 		"\xee\x98\xa0"
#define MY_SYMBOL_TEMPER 			MY_ICON_TEMPER
#define MY_SYMBOL_I 				MY_ICON_CURRENT
#define MY_SYMBOL_EXIT				"\xee\x98\x8e"

#else

//symbol_32
#define MY_SYMBOL_FIBER_CONFIG 		"\xef\x88\x81"
#define MY_SYMBOL_SETTING 			"\xee\x98\xa9"
#define MY_SYMBOL_LD_CONFIG 		"\xee\x98\xa0"
#define MY_SYMBOL_TEMPER 			"\xef\x9d\xa9"
#define MY_SYMBOL_I 				"\xef\x93\xb0"
#define MY_SYMBOL_EXIT	 			"\xef\x8b\xb6"


#endif

/*********************************************************
*					用户基本参数
**********************************************************/

#define MY_DEVICE_NAME		"Faraday Laser"
#define LOGIN_USER			"admin"			//用户名
#define LOGIN_PASSWORD		"12345678"		//密码

#define INIT_PAGE_ID		PAGE_BOOT		//默认启动的第一个应用




#define PAGE_INIT_DEF(TAG)  Gui_##TAG##Init
#define PAGE_FOCUS_DEF(TAG) Gui_##TAG##OnFocus
#define PAGE_EXIT_DEF(TAG) Gui_##TAG##Exit

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

/*********************************************************
*					DFB参数数量
**********************************************************/

#define VIEW_SAMPLE_NUMS				4
#define MAX_TEMP_NUMS					3
#define MAIN_SAMPLE_NUMS				4

#define DEVELOP_ITEM_NUMS_END 			7     //常用参数数量
#define PARAM_ITEM_NUMS_END				15	  //其他参数数量
#define ALARM_OFFSET					7	  //告警值与设置值的序号偏移


#define RELAY_NUMS 						4	  //开关继电器数量
#define DATA_OFFSET						6	  //采集(副)与采集（主）数组偏移


#define RANGE_MIN	45			  			//弧形进度条初始角
#define RANGE_MAX	315			  			//弧形进度条终止角



/*********************************************************
*					光纤激光器参数
**********************************************************/






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

enum E_DFB_ItemIndex
{
	Item_Current   		= 0,
	Item_Temper_Lv1	   	= 1,
	Item_Temper_Lv2	   	= 2,	
	Item_Temper_Lv3	   	= 3,	
	Item_PztAmp	= 4,
	Item_Current_work = 9,
	Item_Temper_Lv1_work	   = 10,
	Item_Temper_Lv2_work	   = 11,	
	Item_Temper_Lv3_work	   = 12,		
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

/*******************************************FIBER**************************************************/	
typedef enum 
{
	_cfg_index_ISet  = 0,
	_cfg_index_Freq,
	_cfg_index_Duty,
	_cfg_index_Max,
}e_cfg_index;

typedef enum 
{
	fiber_mode_DGC  = 0,		//连续模式
	fiber_mode_DGQ,				//准连续模式
	fiber_mode_ANG,				//模拟控制模式
	fiber_mode_max,
}e_fiber_mode;



enum E_FIBER_WarnIndex
{
	Item_Warn_Temp_down	 = 7,
	Item_Warn_PdX  	 	 = 5,
	Item_Warn_Temp_up  	 = 2,
	Item_Warn_LockStat   = 1,	
	Item_Warn_Oher  	 = 0,	
};

enum E_FIBER_PageIndex
{
	Page_Index_BtnArea = 0,
	Page_Index_WarnTempD = 1,		
	Page_Index_WarnPD,
	Page_Index_WarnTempU,
	Page_Index_WarnOther,	
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


typedef struct
{
	lv_obj_t* cont;
	lv_obj_t* lableValue;
	lv_obj_t* lableUnit;
	Modbus_Date *sample;
}SubInfo_t;

typedef struct
{
	lv_obj_t* cont;
	lv_obj_t* Bar_Main;
	lv_obj_t* Bar_icon;
}viewInfo_t;


typedef struct _module_t
{
    lv_obj_t* obj_root;
    lv_obj_t* iconlabel;
    lv_obj_t* usage_label;
    lv_obj_t* usage_chart;
    lv_chart_series_t* usage_series;
	Modbus_Date *mod_sample;	
}module_t;

typedef struct _dialog_t
{
	lv_obj_t* cont;
	lv_obj_t* title;
	lv_obj_t* btnOK;
	uint8_t bCheckFlag;
} dialog_t;	


struct _ui_info
{
	struct
	{
		lv_obj_t* cont;
		module_t* I_module;
		module_t* T_module[MAX_TEMP_NUMS];
	}topInfo;
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
	struct
	{
		bool bFirstEntryLock;
		bool bIsNeedAllFlush;
	} Stat;		
	lv_timer_t *collect_timer;
};




struct m_attr_t
{
	bool bHasDot;
	uint8_t itemIndex;
	uint8_t wholeCnt;		//显示的数字位数
	uint8_t integerCnt;		//显示的整数部分位数
	float accuracy;
	int32_t range_max;
	int32_t range_min;
	int32_t _initVal;
};

typedef struct _ctrl_module
{
	struct m_attr_t _attr;
    lv_obj_t* _root;
    lv_obj_t* _mObj;
}ctrl_module_t;

typedef struct _warn_module
{
	uint8_t bitPos;		//bit位偏移
    uint8_t valPos;		//字节偏移
    lv_obj_t* _iconObj;
}warn_module_t;

typedef struct _sw_module
{
	struct m_attr_t _attr;
    lv_obj_t *_mObj;
	//新增开关可变标签 2024-2-21
	lv_obj_t *_titleLabel;
	lv_anim_t anim;
}sw_module_t;


struct _ui_Setting
{
	bool bIsAdmin;
	bool bIsFirstCd;
	bool bIsEntryCountDwn;	//进入倒计时
	bool bIsFirstPwrOn;
	uint8_t IsOneStepLock;
	int timerCntDown;
    lv_obj_t* _tabCont;
	struct _ctrl_module  *_mods[PARAM_ITEM_NUMS_END];
	struct _ctrl_module  *_mods_sw[RELAY_NUMS];
	struct _sw_module  	*_mods_sp[2];
	lv_obj_t *_ScanObj;
	viewInfo_t labelGrp[VIEW_SAMPLE_NUMS];
	uint32_t iSwitchs;
};
extern struct _ui_Setting _settingUI;



typedef struct _FuncBtnAttr	
{
	int xPos;
	int yPos;
	uint32_t iColor;
	lv_obj_t* ctrlObj;
	void (*funcHandler)(lv_event_t *e);

}M_FuncBtnAttr;


typedef struct _fiberParam 
{
	uint8_t mode;
	uint16_t switchs;	
	uint16_t I_UserMax[2];		//一、二级最大电流
	uint16_t Pd_val[4];
	uint32_t cfg_val[_cfg_index_Max];
	uint8_t warn_val[8];
}M_fiberParam;


struct _Fiber_Setting
{
    lv_obj_t* _taCont;
	struct _ctrl_module  *_mods[FIBER_NUMS_END];		//spin类指针
	struct _warn_module  *_items[FIBER_WARN_NUMS_END];	//警告类
	M_FuncBtnAttr *ptrBtn;								//功能按键类指针
	uint8_t	sample_inx;
	lv_timer_t *sample_timer;
};

extern struct _Fiber_Setting _fiberUI;


typedef struct _enc_flewChk
{
	bool bIsTimerRun;
	bool bIsDataSend;
	bool bIsIncEnabled;
	uint32_t idleCnt;
	uint8_t targetInx;
	lv_timer_t *enc_flewTimer;
}enc_flewChk_t;


struct _login_info
{
	lv_obj_t* cont;
	lv_obj_t* kb;
	lv_obj_t* userInput;
	lv_obj_t* pwdInput;
	lv_obj_t* loginBtn;
};


extern uint8_t anim_reback;



extern struct _ui_info ui;
extern struct _login_info login_win;
extern struct _dialog_t ui_Dialog;

extern lv_style_t style_Dialog;
extern lv_style_t style_Info;
extern lv_style_t style_tabIcon;
extern lv_style_t style_ManualBtn;
extern lv_style_t style_tabContent;
extern lv_style_t style_Window;
extern lv_style_t style_FuncBtn;
extern lv_style_t style_SubPage;


//开机logo字体
LV_FONT_DECLARE(font_Atlantico_22)
//采集显示界面字体
LV_FONT_DECLARE(font_tw_15)
//采集显示界面字体
LV_FONT_DECLARE(font_bahnschrift_17)
LV_FONT_DECLARE(font_bahnschrift_13)
LV_FONT_DECLARE(font_bahnschrift_35)

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

extern void Gui_settingInit(lv_obj_t* root);
extern void Gui_settingOnFocus(lv_obj_t* root);
extern void Gui_settingExit(lv_obj_t* root);

//TA setting
extern void Gui_paramInit(lv_obj_t* root);
extern void Gui_paramOnFocus(lv_obj_t* root);
extern void Gui_paramExit(lv_obj_t* root);


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
extern void sw_flush_val(lv_obj_t *obj, uint8_t index, uint32_t val);


extern bool Gui_CheckTimeOut(lv_timeCount *tm);
extern bool Gui_SetTimeOut(lv_timeCount *tm, uint32_t ms);
extern void Gui_PageCallback(void* arg, bool del);


extern void sample_tile_init(lv_obj_t* root);
extern void sample_tile_exit(void);
extern void Gui_dialog_Create(void);
extern void Gui_menuInit(void);
extern void spinContent_style_init(ctrl_module_t* t_objBox, const char **label_list, lv_event_cb_t event_cb);
extern lv_obj_t *spinBtn_style_init(ctrl_module_t* t_objBox, lv_event_cb_t event_cb);

extern void switchLock_valInit(sw_module_t* t_swMod);

#endif

