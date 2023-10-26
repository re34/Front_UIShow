#include "ui_bar.h"
#include "ui_utils.h"

#include <stdio.h>

static ui_Bar_t *main_bar;
uint8_t time_update = 0;

struct _objId obj_ID;

ui_rtc_t rtc;


static void anim_y_cb(void* var, int32_t v)
{
	lv_obj_set_y(var, v);
}


static void anim_ready_cb(lv_anim_t* a)
{
	lv_anim_del(a->var, NULL);
	//if(main_bar->hide_cb != NULL && !(main_bar->bIsShow))
	//	main_bar->hide_cb();
}


static void ui_bar_SetState(uint8_t type, uint8_t state)
{
	lv_anim_t anim;
	switch(type)
	{
		case CMD_CTRL_BAR:
			if(state)
			{
				if(main_bar->bIsShow)
					break;
				ANIM_LINE_ADD(&anim, anim_y_cb, lv_anim_path_overshoot, anim_ready_cb, 300, 0, 500, main_bar->bar, -32, 0, 0);
			}
			else
			{
				if(!(main_bar->bIsShow))
					break;
				ANIM_LINE_ADD(&anim, anim_y_cb, lv_anim_path_overshoot, anim_ready_cb, 300, 0, 200, main_bar->bar, 0, -32, 0);
			}
			main_bar->bIsShow = state;
			lv_anim_start(&anim);
			break;
		case CMD_SET_TIME:
			time_update = state;
			break;
		default:
			break;
	}
}

static void ui_bar_SetTitle(const char* title)
{
	lv_obj_t* obj = lv_obj_get_child(main_bar->bar, obj_ID.title_id);
	lv_label_set_text(obj, title);
}

static void ui_bar_SetParent(lv_obj_t* parent)
{
	lv_obj_set_parent(main_bar->bar, parent);
}


static void bar_time_task(lv_timer_t *timer)
{
	lv_obj_t* obj = lv_obj_get_child(main_bar->bar, obj_ID.time_id);
	if(++rtc.Sec >= 60)
	{
		rtc.Sec = 0;
		if(++rtc.Min >= 60)
		{
			rtc.Min = 0;
			if(++rtc.Hour >= 24)
				rtc.Hour = 0;
        }
    }

	if(time_update)
	{
		char buff[16];
		sprintf(buff, "%02d : %02d", rtc.Hour, rtc.Min);
		lv_label_set_text(obj, buff);
	}
	else
	{
		lv_label_set_text(obj, MY_DEVICE_NAME);
	}
}


void ui_bar_init(void)
{
	main_bar->bar = lv_obj_create(lv_scr_act());
	lv_obj_set_size(main_bar->bar, 480 - 5, 40);
	lv_obj_set_style_border_width(main_bar->bar, 2, 0);
	lv_obj_set_style_pad_all(main_bar->bar, 0, 0);
	lv_obj_set_style_opa(main_bar->bar, LV_OPA_COVER, 0);
	lv_obj_set_style_border_side(main_bar->bar, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT, 0);
	lv_obj_align(main_bar->bar, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_clear_flag(main_bar->bar, LV_OBJ_FLAG_SCROLLABLE); //È¡Ïû¹ö¶¯


	lv_obj_t* label_time = lv_label_create(main_bar->bar);
	obj_ID.time_id = lv_obj_get_child_id(label_time);
	lv_label_set_text(label_time, "08 : 53");
	lv_obj_set_size(label_time, LV_SIZE_CONTENT, 24);
	lv_obj_set_style_text_color(label_time, lv_color_make(30, 48, 80), 0);
	lv_obj_align(label_time, LV_ALIGN_LEFT_MID, 5, 3);
	lv_memset((void *)&rtc, 0, sizeof(ui_rtc_t));
	rtc.Hour = 8;
	rtc.Min = 53;

	lv_obj_t* label_alarm = lv_label_create(main_bar->bar);
	obj_ID.alarm_id = lv_obj_get_child_id(label_alarm);
	lv_label_set_text(label_alarm, LV_SYMBOL_BELL); 
	lv_obj_set_size(label_alarm, 22, 22);
	lv_obj_set_style_text_color(label_alarm, lv_color_make(30, 48, 80), 0);
	lv_obj_align(label_alarm, LV_ALIGN_RIGHT_MID, -5, 3);


	lv_obj_t* label_sound = lv_label_create(main_bar->bar);
	obj_ID.sound_id = lv_obj_get_child_id(label_sound);
	lv_label_set_text(label_sound, LV_SYMBOL_VOLUME_MID);
	lv_obj_set_size(label_sound, 22, 22);
	lv_obj_set_style_text_color(label_sound, lv_color_make(30, 48, 80), 0);
	lv_obj_align_to(label_sound, label_alarm, LV_ALIGN_OUT_LEFT_MID, -2, 0);

	lv_obj_t* label_title = lv_label_create(main_bar->bar);
	obj_ID.title_id = lv_obj_get_child_id(label_title);
	lv_label_set_text(label_title, "");
	lv_obj_set_size(label_title, LV_SIZE_CONTENT, 24);

	lv_obj_set_style_text_color(label_title, lv_color_make(30, 48, 80), 0);
	lv_obj_align(label_title, LV_ALIGN_CENTER, 0, 3);

	//lv_timer_t* lv_timer =
	lv_timer_create(bar_time_task, 1000, NULL);
}

ui_Bar_t *ui_bar_instance(void)
{
	main_bar = (ui_Bar_t *)rt_malloc(sizeof(ui_Bar_t));
	rt_memset(main_bar, 0, sizeof(ui_Bar_t));
	main_bar->state_set = ui_bar_SetState;
	main_bar->parent_set = ui_bar_SetParent;
	main_bar->title_set = ui_bar_SetTitle;

	ui_bar_init();
	return main_bar;
}

