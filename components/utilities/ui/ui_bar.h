#ifndef __UI_BAR_H
#define __UI_BAR_H

#include "lvgl.h"

typedef struct
{
	lv_obj_t *bar;
	//void(*hide_cb)();
	void(*state_set)(uint8_t type, uint8_t state);
	void(*parent_set)(lv_obj_t* parent);
	void(*title_set)(const char* title);	
	uint8_t bIsShow;
}ui_Bar_t;


typedef struct
{
  uint8_t Year;
  uint8_t Month;
  uint8_t Date;
  uint8_t DaysOfWeek;
  uint8_t Hour;
  uint8_t Min;
  uint8_t Sec;
} ui_rtc_t;

struct _objId
{
	uint16_t time_id:1;
	uint16_t bat_id:1;
	uint16_t alarm_id:2;
	uint16_t sound_id:2;
	uint16_t title_id:3;
};


enum
{
	CMD_CTRL_BAR,
	CMD_SET_ALARM,
	CMD_SET_SOUND,
	CMD_SET_TIME,
};

extern ui_Bar_t *ui_bar_instance(void);

#endif

