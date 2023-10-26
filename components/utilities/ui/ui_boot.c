#include "ui_utils.h"
#include <math.h>


static lv_timeCount time_20ms;
lv_timer_t *timer_handle;
static lv_obj_t *arc[3];

#ifndef PI
#define PI  (3.14159f)
#endif


static void anim_timer_handle(lv_obj_t *root)
{
    static int32_t count = -90;
    lv_obj_t *page = root;
    lv_obj_t *img_logo = NULL;
	lv_obj_t *label = NULL;

    if (-90 == count) {
        img_logo = lv_img_create(page);
        lv_img_set_src(img_logo, "L:/logo2.png");
        lv_obj_align(img_logo, LV_ALIGN_CENTER, 0, -10);
		
		label = lv_label_create(page);
		lv_obj_set_style_text_color(label, lv_color_hex(0x040480), LV_PART_MAIN);
		lv_obj_set_style_text_font(label, &font_Atlantico_22, LV_PART_MAIN);
		lv_label_set_text(label, "FARADAY  LASER");
		lv_obj_align_to(label, img_logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }

    if (count < 90) {
        lv_coord_t arc_start = count > 0 ? (1 - cosf(count / 180.0f * PI)) * 270 : 0;
        lv_coord_t arc_len = (sinf(count / 180.0f * PI) + 1) * 135;

        for (size_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
            lv_arc_set_bg_angles(arc[i], arc_start, arc_len);
            lv_arc_set_rotation(arc[i], (count + 120 * (i + 1)) % 360);
        }
    }
    if (count == 90) {
        for (uint8_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
            lv_obj_del(arc[i]);
        }
    }
	//完成函数跳转
    if ((count += 2) >= 100) 
	{
		lv_timer_enable(false);
		lv_timer_del(timer_handle);
		lv_timer_enable(true);
		uint8_t id = PAGE_MAIN;
		Gui_PageCallback((void *)&id, 1);		
   }
}


static void Gui_boot_timerCb(lv_timer_t *tmr)
{
    if (Gui_CheckTimeOut(&time_20ms)) {
        anim_timer_handle((lv_obj_t *)tmr->user_data);
    }
}


void Gui_bootInit(lv_obj_t* root)
{
    lv_timer_enable(false);

	lv_obj_remove_style_all(root);
	lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(root, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(root, lv_color_white(), LV_PART_MAIN);
	timer_handle = lv_timer_create(Gui_boot_timerCb, TIME_ON_TRIGGER, (void *)root);
}

void Gui_bootOnFocus(lv_obj_t* root)
{
    const lv_color_t arc_color[] = {
        LV_COLOR_MAKE(143, 152, 233),
        LV_COLOR_MAKE(159, 168, 217),
        LV_COLOR_MAKE(124, 188, 252),
    };
    for (uint8_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
        arc[i] = lv_arc_create(root);
        lv_obj_set_size(arc[i], 220 - 30 * i, 220 - 30 * i);
        lv_arc_set_bg_angles(arc[i], 120 * i, 0 + 120 * i);
        lv_arc_set_value(arc[i], 0);
        lv_obj_remove_style(arc[i], NULL, LV_PART_KNOB);
        lv_obj_set_style_arc_width(arc[i], 9, 0);
        lv_obj_set_style_arc_color(arc[i], arc_color[i], 0);
        lv_obj_center(arc[i]);
    }
	Gui_SetTimeOut(&time_20ms, 20);
	lv_timer_enable(true);
}

void Gui_bootExit(lv_obj_t *root)
{
	return;
}

