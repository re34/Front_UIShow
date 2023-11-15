#ifndef __MOD_PARA_H
#define __MOD_PARA_H

#include <stdbool.h>

#include "spi_ad9833.h"
#include "spi_ad5541.h"


#define COLLECT_MAX_NUM  100
#define INI_FILE_DIR 	 "/config/config.ini"

typedef enum
{
	co_string,
	co_bool,
	co_uint8,
	co_int8,
	co_uint16,
	co_int16,
	co_uint32,
	co_int32,
	co_float,
	co_long,
	co_ulong,
	co_type_max,
} comm_data_type;

typedef struct
{
	void *val;
	comm_data_type type;
} comm_val_t;


typedef union
{
	char string[16];
	bool b1;
	uint8_t u8;
	int16_t i16;
	uint16_t u16;
	float f32;
	uint32_t u32;
	long l32;
} union_data_t;


struct ini_data_t
{
	char section_name[16];
	char key_name[24];
	union_data_t def_val;
	unsigned short index;
};


typedef struct _sys_paras
{	
	M_Ad9833Para _tDdsPara;  // dds波形参数
	union _Dac_Para _uScanPara; // 扫描波形参数
}M_sys_paras;


extern struct _sys_paras _g_ParasPool;


extern void Get_SystemParam(void);
extern void Set_SystemParam(uint16_t index);	


#endif

