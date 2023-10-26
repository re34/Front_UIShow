#ifndef __MOD_PARA_H
#define __MOD_PARA_H

#include <stdbool.h>
#include "spi_ad9833.h"


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




typedef struct _DacPara
{
	M_Ad9833Para _tDdsPara;  // dds波形参数
//	M_Ad5541Para _tScanPara; // 扫描波形参数		
}M_DacPara;

typedef struct _paras_typedef
{
	//enum _LockStage _CurLockStage; 			 // 自动锁状态机
	int16_t s_i16ErrArr[COLLECT_MAX_NUM];
	int16_t s_u16PztArr[COLLECT_MAX_NUM];
	int16_t s_i16SasArr[COLLECT_MAX_NUM];	
	struct
	{
		long iCurrent_RealVal; //LD实际电流
		long iCurrent_SetVal;  //LD电流设定值
	}LdPara;
	M_DacPara t_DacParas;
	struct
	{
		uint32_t csv_line_count; // csv当前写入行数记录
		uint32_t _CurSize;		 // 记录用户当前数据写入尺寸
	}RecordFile;
	//void *pTransBus;
}M_paras_typedef;

extern struct _paras_typedef _g_ParasPool;

extern void Get_SystemParam(void);
extern void Set_SystemParam(comm_val_t *pv, uint16_t index);	


#endif

