#include "ui_utils.h"
#include "ui_bar.h"
#include "lv_port_indev.h"

#define APP_COUNT 		3
#define aLenth  		APP_COUNT * 2
#define FIXED_OBJ_ID	2


const char* app_names_symbol[] = {
	MY_SYMBOL_HISTORY,
	MY_SYMBOL_SUMMARY,
	MY_SYMBOL_SETTING,
};

const char* app_names[] = {
    "TA Setting","Summary","LD Setting",
};


lv_anim_t a[aLenth];
static lv_anim_timeline_t* anim_timeline = NULL;
static uint8_t app_index = 0;
uint8_t anim_reback = 0;

static lv_obj_t *icons[APP_COUNT];
static ui_Bar_t *menu_bar = NULL;


ui_locate_t app_locate[] = {
	{0, 2},
	{1, 1},
	{2, 0},
};

static void anim_rGx_cb(void* var, int32_t v)
{
    lv_obj_set_x(var, v);
}


static void anim_dWy_cb(void* var, int32_t v)
{
    lv_obj_set_y(var, v);
}



static uint32_t ui_GetAppIndex(int8_t offset)
{
    return Gui_Basic_GetOffset(app_index, APP_COUNT, offset);
}

static void arc_path_by_theta(int16_t theta, int16_t *x, int16_t *y)
{
    *x =(lv_trigo_sin(theta) * 45) >> LV_TRIGO_SHIFT;
    *y = 12 + ((lv_trigo_sin(theta + 90) * 45) >> LV_TRIGO_SHIFT);
}


void anim_FinishCb(lv_anim_t *a)
{
	lv_obj_t *sub_Obj = (lv_obj_t *)a->var;
	lv_obj_t *parent_obj = lv_obj_get_parent(sub_Obj);
	lv_obj_t *label;


    if(!anim_reback)
    {
        lv_obj_set_style_border_width(parent_obj, 3, 0);
        lv_obj_set_style_border_color(parent_obj, lv_color_hex(0x7d71c3), LV_PART_MAIN);
        lv_obj_set_style_shadow_color(parent_obj, lv_color_hex(0x7d71c3), LV_PART_MAIN);
        Gui_setHightLight(parent_obj, 1);

        for (int i = 0; i < APP_COUNT; i++)
        {
            label = lv_obj_get_child(icons[i], 0);
            if(i == app_index)
            {
                lv_obj_set_style_text_color(label, lv_color_hex(0x7d71c3), LV_PART_MAIN);
                Gui_setHightLight(icons[i], 1);
            }else{
                lv_obj_set_style_text_color(label, lv_color_make(30, 48, 80), LV_PART_MAIN);
                Gui_setHightLight(icons[i], 0);
            }
        }
        if (0 != app_index) {
			
            lv_obj_swap(icons[0], icons[app_index]);
			//得到交换obj后的id
			app_locate[app_index].objId = lv_obj_get_index(icons[app_index]);
			app_locate[0].objId = lv_obj_get_index(icons[0]);
        }
    }
	else{
		//隐藏工具条
		menu_bar->state_set(CMD_CTRL_BAR, 0);
		//输入设备失焦
		lv_group_set_editing(lv_group_get_default(), false);
		lv_group_remove_all_objs(lv_group_get_default());
		lv_indev_enable(ts_indev_obj, true);	
		Gui_PageCallback((void *)&app_index, 1);
    }
}

void ui_switch_focus(uint8_t last_inx, uint8_t curr_inx)
{
	lv_obj_t *label;
	for (int i = 0; i < APP_COUNT; i++)
	{
		label = lv_obj_get_child(icons[i], 0);
		if(i == curr_inx)
		{
			lv_obj_set_style_text_color(label, lv_color_hex(0x7d71c3), LV_PART_MAIN);
			Gui_setHightLight(icons[i], 1);
		}else{
			lv_obj_set_style_text_color(label, lv_color_make(30, 48, 80), LV_PART_MAIN);
			Gui_setHightLight(icons[i], 0);
		}
	}
	lv_obj_swap(icons[last_inx], icons[curr_inx]);
	//得到交换obj后的id
	app_locate[last_inx].objId = lv_obj_get_index(icons[last_inx]);
	app_locate[curr_inx].objId = lv_obj_get_index(icons[curr_inx]);
}




static void ui_MenuEventCb(lv_event_t *e)
{
	uint8_t last_inx;
    lv_event_code_t code = lv_event_get_code(e);
    if (LV_EVENT_KEY == code)
	{
	    uint32_t key = lv_event_get_key(e);
		if (LV_KEY_USER_OK == key)
		{
			lv_indev_wait_release(lv_indev_get_act());
#if defined(UI_USING_PAGE_PARAM) || defined(UI_USING_PAGE_SETTING)
			if(app_index == PAGE_PARAM)	//其余2个编号的app未开放
			{
				anim_reback = 1;
				lv_anim_timeline_set_reverse(anim_timeline, anim_reback);
				lv_anim_timeline_start(anim_timeline);
			}
#endif
		}else{
			last_inx = app_index;
	        if (LV_KEY_RIGHT == key) {
	            app_index = ui_GetAppIndex(-1);
	        } else if (LV_KEY_LEFT == key) {
	            app_index = ui_GetAppIndex(1);
	        }
			menu_bar->title_set(app_names[app_index]);
			ui_switch_focus(last_inx, app_index);
		}
    }else if (LV_EVENT_FOCUSED == code) {
		lv_group_set_editing(lv_group_get_default(), true);
	}
	else if (LV_EVENT_CLICKED == code)
	{
		lv_indev_wait_release(lv_indev_get_act());
		lv_obj_t *ponit_obj = lv_event_get_target(e);
		uint8_t id = lv_obj_get_index(ponit_obj);
		if(id != FIXED_OBJ_ID)//选中的不是已聚焦的id(聚焦的obj的id是固定的)
		{
			for(int i = 0; i < 3; i++)
			{
				if(app_locate[i].objId == id)
				{
					app_index = app_locate[i].inx;
				}
				if(app_locate[i].objId == FIXED_OBJ_ID)
				{
					last_inx = app_locate[i].inx;
				}
			}
			menu_bar->title_set(app_names[app_index]);
			ui_switch_focus(last_inx, app_index);
		}else{
#if defined(UI_USING_PAGE_PARAM) || defined(UI_USING_PAGE_SETTING)
			if(app_index == PAGE_PARAM)  //PAGE_SUMMARY app未开放
			{
				//禁止触屏，防止动画过程中误触情况发生
				lv_indev_enable(ts_indev_obj, false);
				anim_reback = 1;
				lv_anim_timeline_set_reverse(anim_timeline, anim_reback);
				lv_anim_timeline_start(anim_timeline);
			}
#endif
		}

	}
}



void Gui_mainOnFocus(lv_obj_t* root)
{
	int16_t x, y;
	int16_t i = 0, j = 0;

    for (i = APP_COUNT - 1; i >= 0; i--)
	{
		lv_obj_t* label;

		icons[i] = lv_obj_create(root);
		lv_obj_set_size(icons[i], 90, 90);
		lv_obj_set_style_radius(icons[i], LV_PCT(50), LV_PART_MAIN);
		lv_obj_clear_flag(icons[i], LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_set_style_shadow_color(icons[i], lv_color_hex(0x7d71c3), LV_PART_MAIN);
		lv_obj_align(icons[i], LV_ALIGN_CENTER, 0, 12);

		label = lv_label_create(icons[i]);
		lv_obj_set_style_text_color(label, lv_color_make(30, 48, 80), LV_PART_MAIN);
		lv_obj_set_style_text_font(label, &font_symbol_32, LV_PART_MAIN);
		lv_label_set_text(label, app_names_symbol[i]);
		lv_obj_center(label);
		lv_obj_add_event_cb(icons[i], ui_MenuEventCb, LV_EVENT_KEY, NULL);
		lv_obj_add_event_cb(icons[i], ui_MenuEventCb, LV_EVENT_CLICKED, NULL);

		arc_path_by_theta(180 + i * 120, &x, &y);
		lv_anim_init(&a[j]);
		lv_anim_set_var(&a[j], icons[i]);
		lv_anim_set_exec_cb(&a[j], anim_rGx_cb);
		lv_anim_set_values(&a[j], 0, x);
		lv_anim_set_path_cb(&a[j], lv_anim_path_overshoot);
		lv_anim_set_time(&a[j], 1000);
		j++;
		lv_anim_init(&a[j]);
		lv_anim_set_var(&a[j], icons[i]);
		lv_anim_set_exec_cb(&a[j], anim_dWy_cb);
		lv_anim_set_values(&a[j], 0, y);
		lv_anim_set_path_cb(&a[j], lv_anim_path_overshoot);
		lv_anim_set_time(&a[j], 1000);
		j++;
		if(j == aLenth)
			lv_anim_set_ready_cb(&a[j - 1], anim_FinishCb);

	}
    anim_timeline = lv_anim_timeline_create();
    for (i = 0; i < aLenth; i++) {
        lv_anim_timeline_add(anim_timeline, 0, &a[i]);
    }
    lv_anim_timeline_start(anim_timeline);
}

void Gui_mainInit(lv_obj_t *root)
{
	//1. app选择主界面
	lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_size(root, 360, 190);
	lv_obj_set_style_pad_all(root, 0, 0);
	lv_obj_set_style_radius(root, LV_RADIUS_CIRCLE, LV_PART_MAIN);
	lv_obj_align(root, LV_ALIGN_CENTER, 0, 15);

    lv_obj_add_event_cb(root, ui_MenuEventCb, LV_EVENT_KEY, NULL);
	lv_obj_add_event_cb(root, ui_MenuEventCb, LV_EVENT_FOCUSED, NULL);
	//加入到旋转编码器
	Gui_AddToIndevGroup(root);
	//添加状态栏
	if(menu_bar == NULL)
		menu_bar = ui_bar_instance();
	menu_bar->state_set(CMD_CTRL_BAR, 1);
	menu_bar->title_set(app_names[app_index]);
	//登录框
#if defined(UI_USING_FUNC_LOGIN)	
	Gui_loginInit();
#endif
}


void Gui_mainExit(lv_obj_t *root)
{
	lv_anim_timeline_stop(anim_timeline);
	lv_anim_timeline_del(anim_timeline);

	for (uint8_t i = 0; i < sizeof(icons) / sizeof(icons[0]); i++) {
		lv_obj_del(icons[i]);
	}
#if defined(UI_USING_FUNC_LOGIN)	
	Gui_loginExit();
#endif
}


