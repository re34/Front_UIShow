#include "ui_utils.h"

#define APP_NUM 3


static lv_obj_t *page;
static uint8_t app_index = 0;
static lv_obj_t *icons[APP_NUM];
static lv_timeCount time_500ms;

static ui_app_t menu[] = {
    {"Washing",     "L:/P1.png", "L:/P1_miss.png", LV_COLOR_MAKE(36, 163, 235)},
    {"Thermostat",  "L:/P2.png", "L:/P2_miss.png", LV_COLOR_MAKE(249, 139, 122)},
    {"Light",       "L:/P3.png", "L:/P3_miss.png", LV_COLOR_MAKE(255, 229, 147)},
};


static bool desktop_layer_enter_cb(struct _lv_layer *layer);
static bool desktop_layer_exit_cb(struct _lv_layer *layer);
static void desktop_layer_timer_cb(lv_timer_t *tmr);


lv_layer_t desktop_Layer = {
    .lv_obj_name    = "desktop_Layer",
    .lv_obj_parent  = NULL,
    .lv_obj_layer   = NULL,
    .lv_show_layer  = NULL,
    .enter_cb       = desktop_layer_enter_cb,
    .exit_cb        = desktop_layer_exit_cb,
    .timer_cb       = desktop_layer_timer_cb,
};


static uint32_t ui_GetAppIndex(int8_t offset)
{
    return ui_Basic_GetOffset(app_index, APP_NUM, offset);
}


static void arc_path_by_theta(int16_t theta, int16_t *x, int16_t *y)
{
    const int ox = 0, oy = 0;
    *x = ox + ((lv_trigo_sin(theta) * 45) >> LV_TRIGO_SHIFT);
    *y = oy + ((lv_trigo_sin(theta + 90) * 45) >> LV_TRIGO_SHIFT);
}

static void obj_set_to_hightlight(lv_obj_t *obj, bool enable)
{
    if (enable) {
        lv_obj_set_style_shadow_width(obj, 15, 0);
        lv_obj_set_style_shadow_spread(obj, 3, 0);
    } else {
        lv_obj_set_style_shadow_width(obj, 0, 0);
        lv_obj_set_style_shadow_spread(obj, 0, 0);
    }
}


static void ui_MenuEventCb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (LV_EVENT_FOCUSED == code) {
        lv_group_set_editing(lv_group_get_default(), true);
    } else if (LV_EVENT_KEY == code) {
        uint32_t key = lv_event_get_key(e);
        if (check_timeOut(&time_500ms)) 
		{
            int8_t last_index = app_index;
            if (LV_KEY_RIGHT == key) {
                app_index = ui_GetAppIndex(-1);
            } else if (LV_KEY_LEFT == key) {
                app_index = ui_GetAppIndex(1);
            }
            for (int i = 0; i < APP_NUM; i++) {
                obj_set_to_hightlight(icons[i], i == app_index);
            }
            lv_obj_swap(icons[last_index], icons[ui_GetAppIndex(0)]);
            lv_img_set_src(icons[last_index], menu[last_index].iconMiss_Path);
            lv_img_set_src(icons[ui_GetAppIndex(0)], menu[ui_GetAppIndex(0)].iconFocu_Path);
            lv_obj_set_style_border_color(page, menu[ui_GetAppIndex(0)].theme_color, 0);			
        }
    }
}

void ui_menu_init(lv_obj_t *parent)
{
    page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);

    lv_obj_set_style_border_width(page, 5, 0);
    lv_obj_set_style_border_color(page, menu[ui_GetAppIndex(0)].theme_color, 0); //默认选中第一个图标
    lv_obj_set_style_radius(page, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(page);
    lv_obj_refr_size(page);

    for (int i = APP_NUM - 1; i >= 0; i--) 
	{
        int16_t x, y;
        arc_path_by_theta(180 + i * 120, &x, &y);
        icons[i] = lv_img_create(page);
        if (i == app_index) {
            lv_img_set_src(icons[i], menu[i].iconFocu_Path);
        } else {
            lv_img_set_src(icons[i], menu[i].iconMiss_Path);
        }
        lv_obj_align(icons[i], LV_ALIGN_CENTER, x, y);
        lv_obj_set_style_border_color(icons[i], menu[i].theme_color, 0);
        lv_obj_set_style_shadow_color(icons[i], menu[i].theme_color, 0);
        lv_obj_set_style_border_width(icons[i], 2, 0);
        lv_obj_set_style_radius(icons[i], LV_RADIUS_CIRCLE, 0);
        obj_set_to_hightlight(icons[i], i == app_index);
    }
    if (0 != app_index) {
        lv_obj_swap(icons[0], icons[app_index]);
    }
#if 0
    label1 = lv_label_create(page);
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_20, 0);
    lv_label_set_text(label1, menu[app_index].name);
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_BOTTOM_MID, 0, -6);
#endif
    lv_obj_add_event_cb(page, ui_MenuEventCb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(page, ui_MenuEventCb, LV_EVENT_KEY, NULL);
	//加入到旋转编码器
	ui_AddToEncoderGroup(page);
}



static bool desktop_layer_enter_cb(struct _lv_layer *layer)
{
    bool ret = false;
    LV_LOG_USER("lv_obj_name:%s, screen:[%d, %d]", layer->lv_obj_name, LV_HOR_RES, LV_VER_RES);
    if (NULL == layer->lv_obj_layer) 
	{
        ret = true;
        layer->lv_obj_layer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(layer->lv_obj_layer, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_border_width(layer->lv_obj_layer, 0, 0);
        lv_obj_set_style_pad_all(layer->lv_obj_layer, 0, 0);

        ui_menu_init(layer->lv_obj_layer);
    }
    set_timeOut(&time_500ms, 500);
    return ret;
}
static bool desktop_layer_exit_cb(struct _lv_layer *layer)
{
    return true;
}

static void desktop_layer_timer_cb(lv_timer_t *tmr)
{
    return;
}

