#ifndef __UI_UTILS_H
#define __UI_UTILS_H

#include <rtthread.h>
#include <lvgl.h>

typedef struct _lv_layer
{
    char *lv_obj_name;
    lv_obj_t *lv_obj_parent;
    lv_obj_t *lv_obj_layer;
    struct _lv_layer *lv_show_layer;
    bool (*enter_cb)(struct _lv_layer *layer);
    bool (*exit_cb)(struct _lv_layer *layer);
    lv_timer_cb_t timer_cb;
    lv_timer_t *timer_handle;
}lv_layer_t;

typedef struct {
    uint32_t time_base;
    uint32_t timeOut;
}lv_timeCount;


typedef struct {
    const char *name;
    const char *iconFocu_Path;
    const char *iconMiss_Path;
    lv_color_t theme_color;
} ui_app_t;



extern lv_layer_t desktop_Layer;
extern lv_layer_t boot_Layer;


extern uint32_t ui_Basic_GetOffset(uint32_t num, int32_t max, int32_t offset);
extern rt_bool_t check_timeOut(lv_timeCount *tm);
extern rt_bool_t set_timeOut(lv_timeCount *tm, uint32_t ms);
extern void lv_boot_create(void);
extern void lv_func_goto_layer(lv_layer_t *dst_layer);


extern void ui_AddToEncoderGroup(lv_obj_t *obj);
extern void ui_BootInit(void);

#endif

