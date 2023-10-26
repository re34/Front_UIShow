#include "ui_utils.h"

#include <math.h>



static lv_timeCount time_20ms;
static lv_obj_t *arc[3];

#ifndef PI
#define PI  (3.14159f)
#endif


static bool boot_layer_enter_cb(struct _lv_layer *layer);
static bool boot_layer_exit_cb(struct _lv_layer *layer);
static void boot_layer_timer_cb(lv_timer_t *tmr);

lv_layer_t boot_Layer = {
    .lv_obj_name    = "boot_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = boot_layer_enter_cb,
    .exit_cb        = boot_layer_exit_cb,
    .timer_cb       = boot_layer_timer_cb,
};


static void anim_timer_handle(lv_obj_t *parent)
{
    static int32_t count = -90;
    lv_obj_t *page = parent;
    static lv_obj_t *img_logo = NULL;
	
    if (-90 == count) {
        img_logo = lv_img_create(page);
        lv_img_set_src(img_logo, "L:/logo2.png");
        lv_obj_center(img_logo);
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
        for (size_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
            lv_obj_del(arc[i]);
        }
    }

    if ((count >= 100) && (count <= 180)) {
        lv_coord_t offset = (sinf((count - 140) * 2.25f / 90.0f) + 1) * 15.0f;
    }
    if ((count += 2) >= 100) {
        lv_func_goto_layer(&desktop_Layer);
    }
}



//启动动画
void boot_animate_start(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(parent, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(parent, lv_color_make(0xFF, 0x0, 0x0), 0);

    const lv_color_t arc_color[] = {
        LV_COLOR_MAKE(143, 152, 233),
        LV_COLOR_MAKE(159, 168, 217),
        LV_COLOR_MAKE(124, 188, 252),
    };
    for (size_t i = 0; i < sizeof(arc) / sizeof(arc[0]); i++) {
        arc[i] = lv_arc_create(parent);
        lv_obj_set_size(arc[i], 220 - 30 * i, 220 - 30 * i);
        lv_arc_set_bg_angles(arc[i], 120 * i, 0 + 120 * i);
        lv_arc_set_value(arc[i], 0);
        lv_obj_remove_style(arc[i], NULL, LV_PART_KNOB);
        lv_obj_set_style_arc_width(arc[i], 9, 0);
        lv_obj_set_style_arc_color(arc[i], arc_color[i], 0);
        lv_obj_center(arc[i]);
    }
}

static bool boot_layer_enter_cb(struct _lv_layer *layer)
{
    bool ret = false;
	
    if (NULL == layer->lv_obj_layer) 
		{
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(layer->lv_obj_layer);
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);

        boot_animate_start(layer->lv_obj_layer);
        set_timeOut(&time_20ms, 20);
    }
    return ret;
}

static bool boot_layer_exit_cb(struct _lv_layer *layer)
{
    return true;
}

static void boot_layer_timer_cb(lv_timer_t *tmr)
{
    if (check_timeOut(&time_20ms)) {
        anim_timer_handle(boot_Layer.lv_obj_layer);
    }
}


