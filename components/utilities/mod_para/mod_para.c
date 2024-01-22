
#include <rtthread.h>
#include "minIni.h"
#include "mod_para.h"
#include <stdio.h>

#if defined(RT_USING_USER_PARA)
#define __INIT_INI_VAL(_sn, _kn, _v_name, _dv, _in)                   \
	{                                                                 \
		.section_name = _sn, .key_name = _kn, .def_val._v_name = _dv, \
		.index = _in                                                  \
	}

#define BY_DATA_TYPE_GET_SIZE(_type)                                                                                                \
	(_type) > co_string && (_type)<co_uint16 ? 1U : (_type)> co_int8 && (_type)<co_uint32 ? 2U : (_type)> co_type_max ? co_type_max : 4U


const char *text_name[co_type_max + 1U] = {
    [co_string] = "string",
    [co_bool] = "bool",
    [co_uint8] = "uint8_t",
    [co_int8] = "int8_t",
    [co_uint16] = "uint16_t",
    [co_int16] = "int16_t",
    [co_uint32] = "uint32_t",
    [co_int32] = "int32_t",
    [co_float] = "float",
    [co_long] = "long",
    [co_ulong] = "ulong",
    [co_type_max] = "null",
};

static struct ini_data_t ini_val_group[] = {
	__INIT_INI_VAL("dds", "frequency", u32, 0, 0x00),
    __INIT_INI_VAL("dds", "frequency register", u8, 0, 0x01),
    __INIT_INI_VAL("dds", "phase", u16, 0, 0x02),
    __INIT_INI_VAL("dds", "phase register", u8, 0, 0x03),
    __INIT_INI_VAL("dds", "wave", u8, 0, 0x04),
    __INIT_INI_VAL("dds", "chipID", u8, 0, 0x05),
    /****************************************************************/
    __INIT_INI_VAL("HvPzt", "bias", u8, 0, 0x06),
    __INIT_INI_VAL("HvPzt", "amp", u16, 0, 0x07),
    __INIT_INI_VAL("HvPzt", "frequency", u8, 0, 0x08),
	__INIT_INI_VAL("HvPzt", "stat", u32, 0, 0x09),    
    __INIT_INI_VAL("HvPzt", "chipID", u8, 0, 0x0a),
    __INIT_INI_VAL("HvPzt", "lockstat", u8, 0, 0x0b),    
};

#define INI_VAL_NUM() (sizeof(ini_val_group) / sizeof(ini_val_group[0]))


//参数 = 实际设定值 x 1000 (除PZT频率外)
struct _sys_paras _g_ParasPool = 
{
#ifdef RT_USING_AD9833
    ._tDdsPara = {
        .frequency = 0,
        .freq_sfr = 0,
        .phase = 0,
        .phase_sfr = 0,
        .wave_mode = Wave_Type_Sin,
        .chipID = 0,
    },
#endif
#ifdef RT_USING_AD5541
#ifdef USING_HVPZT_SCAN
    ._uScanPara = {
		.params[0] = 7000,			
		.params[1] = 7000,
		.params[2] = 10,
		.params[3] = State_In_ScanClose,
		.params[4] = ChipID_HvScan_DecBias,
		.params[5] = Lock_In_StartOn,			
	}
#else
    ._uScanPara = {
		.params[0] = State_In_ScanClose,
		.params[1] = 32700,			
		.params[2] = 32700,
		.params[3] = 10,
		.params[4] = 0,
	}
#endif
#endif
};




#define DBG_BUFF_MAX_LEN          256

/* debug print : support float double */
int dbg_printf(const char *fmt, ...)
{
    va_list args;
    static char rt_log_buf[DBG_BUFF_MAX_LEN] = { 0 };

    va_start(args, fmt);
    int length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);

    rt_kputs(rt_log_buf);

    return length;
}


comm_val_t *Get_ParaDataType(uint16_t index)
{
    /*所有变量中可配置参数映射到该数组中*/
#define Para_Obj &_g_ParasPool
#define VAL(_x) (Para_Obj._x)

    static comm_val_t struct_val_table[] = {
        {VAL(_tDdsPara.frequency), co_uint32},
        {VAL(_tDdsPara.freq_sfr), co_uint8},
        {VAL(_tDdsPara.phase), co_uint16},
        {VAL(_tDdsPara.phase_sfr), co_uint8},
        {VAL(_tDdsPara.wave_mode), co_uint8},
        {VAL(_tDdsPara.chipID), co_uint8},
/****************************************************************/
        {VAL(_uScanPara.params[0]), co_uint32},
        {VAL(_uScanPara.params[1]), co_uint32},
        {VAL(_uScanPara.params[2]), co_uint32},
        {VAL(_uScanPara.params[3]), co_uint32},
        {VAL(_uScanPara.params[4]), co_uint32},
        {VAL(_uScanPara.params[5]), co_uint32},        
    };
#define STRUCT_VAL_NUM() (sizeof(struct_val_table) / sizeof(struct_val_table[0]))
    if (index < STRUCT_VAL_NUM())
        return &struct_val_table[index];
#undef Para_Obj
    return NULL;
}


/*******************************************************************************************************************/

/**
 * @brief	写入数据到.ini文件
 * @details
 * @param	pi ini文件中数据句柄
 * @param   pv 变量结构
 * @retval  none
 */
static void soft_WriteIniFile(struct ini_data_t *pIni, comm_val_t *pv)
{
    int code;
    if (NULL == pIni || NULL == pv)
        return;

    void *pdata = pv->val;
    const char *p_name = text_name[pv->type];

    switch (pv->type)
    {
	    case co_string:
	        code = ini_puts(pIni->section_name, pIni->key_name, (const char *)pdata, INI_FILE_DIR);
	        rt_kprintf("[R -> S] section: %s,val: %s,[%s]: %s.\n", pIni->section_name, pIni->key_name, p_name, (char *)pdata);
	        break;
	    case co_bool:
	    case co_uint8:
	    case co_int16:
	    case co_uint16:
	    case co_int32:
	    case co_uint32:
	    case co_long:
	    case co_ulong:
	    {
	        uint8_t c_size = BY_DATA_TYPE_GET_SIZE(pv->type);
	        long temp_data = 0;
			
	        memcpy(&temp_data, pdata, c_size);
	        code = ini_putl(pIni->section_name, pIni->key_name, temp_data, INI_FILE_DIR);
	        rt_kprintf("[R -> S] section: %s,val: %s,[%s]: %ld.\n", pIni->section_name, pIni->key_name, p_name, temp_data);
	    }
	    break;
	    case co_float:
	        code = ini_putf(pIni->section_name, pIni->key_name, *(float *)pdata, INI_FILE_DIR);
	        dbg_printf("[R -> S] section: %s,val: %s,[%s]: %d.\n", pIni->section_name, pIni->key_name, p_name, *(float *)pdata);
	        break;
	    default:
	        break;
    }

    if (code)
    {
        rt_kprintf("@note: Data written successfully.");
    }
    else
    {
        rt_kprintf("@note: Data write failed,code: %d.", code);
    }
}


static void soft_ReadIniFile(struct ini_data_t *pIni, comm_val_t *pv, uint16_t str_size)
{
    if (NULL == pIni || NULL == pv)
        return;

    const char *pTypeName = text_name[pv->type];
    void *ptr = NULL;
    uint8_t c_size = BY_DATA_TYPE_GET_SIZE(pv->type);
    void *pdata = pv->val;

    switch (pv->type)
    {
	    case co_string:
	        if(str_size)
	            ini_gets(pIni->section_name, pIni->key_name, pIni->def_val.string, (char *)pdata, str_size, INI_FILE_DIR);
	        rt_kprintf("[R <- S] section: %s,val: %s,[%s]: %s.\n", pIni->section_name, pIni->key_name, pTypeName, (char *)pdata);
	        break;
	    case co_bool:
	    {
	        bool data = ini_getbool(pIni->section_name, pIni->key_name, pIni->def_val.b1, INI_FILE_DIR);
	        ptr = &data;
	        rt_kprintf("[R <- S] section: %s,val: %s,[%s]: %d.\n", pIni->section_name, pIni->key_name, pTypeName, data);
	    }
	    break;
	    case co_uint8:
	    case co_int16:
	    case co_uint16:
	    case co_int32:
	    case co_uint32:
	    case co_long:
	    case co_ulong:
	    {
	        long data = ini_getl(pIni->section_name, pIni->key_name, pIni->def_val.l32, INI_FILE_DIR);
	        ptr = &data;
	        rt_kprintf("[R <- S] section: %s,val: %s,[%s]: %ld.\n", pIni->section_name, pIni->key_name, pTypeName, data);
	    }
	    break;
	    case co_float:
	    {
	        float data = ini_getf(pIni->section_name, pIni->key_name, (float)pIni->def_val.f32, INI_FILE_DIR);
	        ptr = &data;
	        dbg_printf("[R <- S] section: %s,val: %s,[%s]: %f.\n", pIni->section_name, pIni->key_name, pTypeName, data);
	    }
	    break;
	    default:
	        break;
    }
    if (ptr && c_size < co_type_max)
        memcpy(pdata, ptr, c_size);
}


/**
 * @brief  获取目标变量在'ini_val_group’位置
 * @param  index 索引值
 * @retval None
 */
struct ini_data_t *soft_GetIniTarget(uint16_t index)
{
    for (struct ini_data_t *pIni = ini_val_group;(pIni < ini_val_group + INI_VAL_NUM()); ++pIni)
    {
        if (pIni->index == index)
        {
            return pIni;
        }
    }
    return NULL;
}

/*******************************************************************************************************************/

#if defined(USING_WRITE_PARAM_CHECK)
/**
 * @brief	检查系统参数的变化
 * @details
 * @param	pi ini文件中数据句柄
 * @param   pv 变量结构
 * @retval  none
 */
bool Chk_SystemParam(struct ini_data_t *pIni, comm_val_t *pv)
{
    bool result = false;

    if (NULL == pIni || NULL == pv)
        return result;

    union_data_t cur_data;
    uint8_t c_size = BY_DATA_TYPE_GET_SIZE(pv->type);	
    memset(&cur_data, 0x00, sizeof(cur_data));
	
    comm_val_t c_val = {
        .val = &cur_data.string,
        .type = pv->type,
    };

    /*先读取文件中目标参数：若本次修改与上次不同，更新.ini*/
    soft_ReadIniFile(pIni, &c_val, 0);
    switch (pv->type)
    {
	    case co_string:
	    {
	        if (strcmp(cur_data.string, (const char *)pv->val))
	            result = true;
	    }
	    break;
	    case co_bool:
	    case co_uint8:
	    case co_int16:
	    case co_uint16:
	    case co_int32:
	    case co_uint32:
	    case co_long:
	    case co_ulong:
	    {
	        long data = 0;
	        memcpy(&data, pv->val, c_size);
	        LOG_D("@note: data: %ld, l32: %ld.", data, cur_data.l32);
	        if (cur_data.l32 != data)
	            result = true;
	    }
	    break;
	    case co_float:
	    {
	        float data = *(float *)pv->val;
	        if (cur_data.l32 != data)
	            result = true;
	    }
	    break;
	    default:
	        break;
    }
    return result;
}
#endif

void Get_SystemParam(void)
{
    for (struct ini_data_t *pIni = ini_val_group; pIni && pIni < ini_val_group + INI_VAL_NUM(); ++pIni)
    {
        comm_val_t *pv = Get_ParaDataType(pIni->index);
        if (pv)
            soft_ReadIniFile(pIni, pv, 0);
    }
}


void Set_SystemParam(uint16_t index)
{
    struct ini_data_t *pIni = soft_GetIniTarget(index);
    comm_val_t *pv = Get_ParaDataType(index);	
	
    if (NULL == pIni || NULL == pv)
        return;

#if defined(USING_WRITE_PARAM_CHECK)
    if (Chk_SystemParam(pIni, pv))
#endif
        soft_WriteIniFile(pIni, pv);
#if defined(USING_WRITE_PARAM_CHECK)
    else
    {
        LOG_I("@note: The current parameters have not changed.");
    }
#endif
}
#endif
