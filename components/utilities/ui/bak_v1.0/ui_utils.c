#include "ui_utils.h"


#define TIME_ON_TRIGGER    10

static lv_layer_t *current_layer = NULL;
static lv_group_t *group;


uint32_t ui_Basic_GetOffset(uint32_t num, int32_t max, int32_t offset)
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

rt_bool_t check_timeOut(lv_timeCount *tm)
{
    int32_t isTmOut;
    isTmOut = (lv_tick_get() - (tm->time_base + tm->timeOut));
    if ((isTmOut > 0) || (0 == tm->time_base)) {
        tm->time_base = lv_tick_get();
        return RT_TRUE;
    } else {
        return RT_FALSE;
    }
}

rt_bool_t set_timeOut(lv_timeCount *tm, uint32_t ms)
{
    tm->time_base = lv_tick_get();
    tm->timeOut = ms;
    return RT_TRUE;
}

void lv_func_create_layer(lv_layer_t *create_layer)
{
    bool result = false;
    result = create_layer->enter_cb(create_layer);
    if (true == result) {
        LV_LOG_INFO("[+] Create lv_layer:%s", create_layer->lv_obj_name);
    }

    if ((true == result) && (NULL == create_layer->timer_handle)) {
        create_layer->timer_handle = lv_timer_create(create_layer->timer_cb, TIME_ON_TRIGGER, NULL);
        LV_LOG_INFO("[+] Create lv_timer:%s", create_layer->lv_obj_name);
    }

    if (create_layer->lv_show_layer) 
	{
        create_layer->lv_show_layer->lv_obj_parent = create_layer->lv_obj_layer;
        result = create_layer->lv_show_layer->enter_cb(create_layer->lv_show_layer);
        if (true == result) {
            LV_LOG_INFO("[+] Create show lv_layer:%s", create_layer->lv_show_layer->lv_obj_name);
        }
        if ((true == result) && (NULL == create_layer->lv_show_layer->timer_handle)) 
				{
            create_layer->lv_show_layer->timer_handle = lv_timer_create(create_layer->lv_show_layer->timer_cb, TIME_ON_TRIGGER, NULL);
            LV_LOG_INFO("[+] Create show lv_timer:%s", create_layer->lv_show_layer->lv_obj_name);
        }
    }
}

void lv_func_goto_layer(lv_layer_t *dst_layer)
{
    lv_timer_enable(false);
    lv_layer_t *src_layer = current_layer;
	//逐级删除目前显示的对象，切换为新的对象
    if (src_layer) {
        if (src_layer->lv_obj_layer) {
            if (src_layer->lv_show_layer) {
                src_layer->exit_cb(src_layer->lv_show_layer);
                LV_LOG_INFO("[-] Delete show lv_layer:%s", src_layer->lv_show_layer->lv_obj_name);
			    //异步删除，它会在下一个 lv_task_handler 调用时被执行
                lv_obj_del_async(src_layer->lv_show_layer->lv_obj_layer);
                src_layer->lv_show_layer->lv_obj_layer = NULL;

                if (src_layer->lv_show_layer->timer_handle) {
                    LV_LOG_INFO("[-] Delete show lv_timer:%s", src_layer->lv_show_layer->lv_obj_name);
                    lv_timer_del(src_layer->lv_show_layer->timer_handle);
                    src_layer->lv_show_layer->timer_handle = NULL;
                }
            }
            src_layer->exit_cb(src_layer);
            LV_LOG_INFO("[-] Delete lv_layer :%s", src_layer->lv_obj_name);
            lv_obj_del_async(src_layer->lv_obj_layer);
            src_layer->lv_obj_layer = NULL;
        }
        if (src_layer->timer_handle) {
            LV_LOG_INFO("[-] Delete lv_timer :%s", src_layer->lv_obj_name);
            lv_timer_del(src_layer->timer_handle);
            src_layer->timer_handle = NULL;
        }
    }

    if (dst_layer) {
        if (NULL == dst_layer->lv_obj_layer) {
            lv_func_create_layer(dst_layer);
        } else {
            LV_LOG_INFO("%s != NULL", dst_layer->lv_obj_name);
        }
        current_layer = dst_layer;
    }
    lv_timer_enable(true);
}


void ui_AddToEncoderGroup(lv_obj_t *obj)
{
    lv_group_add_obj(group, obj);
}


void ui_BootInit(void)
{
	lv_indev_t *indev = NULL;

	group = lv_group_create();
	lv_group_set_default(group);
	while(1)
	{
		indev = lv_indev_get_next(indev);
		if (LV_INDEV_TYPE_ENCODER != lv_indev_get_type(indev)) 
		{
			continue;
		}else{
			LV_LOG_USER("add group for encoder");
			lv_indev_set_group(indev, group);
			lv_group_focus_freeze(group, false);
			break;
		}
	}
	lv_func_goto_layer(&boot_Layer);
}

