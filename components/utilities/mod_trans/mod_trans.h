#ifndef __MOD_TRANS_H
#define __MOD_TRANS_H

#include <rtthread.h>


#define E_Modbus_Write		0x22
#define E_Modbus_Read		0x33
#define E_Modbus_TA_Write	0x34
#define E_Modbus_TA_Read	0x35


#define RT_MODBUS_SIZE   	128


//DFB
#define LD_UART_NAME 		"uart6"
#define LOCK_STATE_ADDR 	5
#define MODBUS_DFB_CFG_ADDR	20 	 //DFB配置起始寄存器地址
#define MODBUS_DFB_CFG_I	22
#define ITEM_ADDR_HEAD		MODBUS_DFB_CFG_ADDR + 2
#define MAX_SAMPLE_NUM 		10
#define MAX_CONFIG_NUM		16	 //switch + 15 spinbox

//7 ~ 8 不需要保存状态
#define BIT_UNLOCK			8
#define BIT_SAVE			7
#define BIT_SCAN			6
#define BIT_AUTO_LOCK		5
#define BIT_PowerSw			4
/******************************************************************************/
//TA
#define TA_UART_NAME 				"uart2"
#define MODBUS_TA_CFG_ADDR			1	 //ta配置起始寄存器地址
#define MODBUS_TA_SAVE_ADDR			15	 //ta保存起始寄存器地址
#define MODBUS_TA_SAMPLE_ADDR		17	 //ta采样起始寄存器地址
#define MODBUS_TA_SAMPLE_CNT		4	 //ta采样值寄存器数量
#define MAX_TA_NUM					12	 //ta设置项+ 保存 + 采样 
//TA参数数量
#define TA_I_NUMS_END 				3     //电流参数
#define TA_T_NUMS_END				7	  //温度参数
#define TA_SAMPLE_ADDR				8	 //ta采样起始数组地址



typedef  union _Modbus_Date {
    uint32_t recvDate;
    struct {
        uint16_t low;					
		uint16_t hi; 					
    } word;
} Modbus_Date;


typedef struct
{	
	uint8_t i_addr;	
	uint8_t i_regNum;	
	uint8_t i_msgType;				
	Modbus_Date w_data;
}M_UartMsgEvent;


extern rt_mq_t uart_mq;
extern struct rt_semaphore g_tErrSem;

extern Modbus_Date ui_sample[MAX_SAMPLE_NUM];
extern Modbus_Date ui_cfgVal[MAX_CONFIG_NUM];
extern Modbus_Date ui_taVal[MAX_TA_NUM];

extern void trans_modbusInit(void);

#endif

