#include "mod_trans.h"
#include "small_modbus.h"
#include "board.h"

#include "lvgl.h"
#include "ui_utils.h"
//串口3    rts PH8
//#define RS485_RST_PIN    GET_PIN(H, 8)


//串口4    rts PC12
#define RS485_RTS_PIN    GET_PIN(C, 12)

static small_modbus_t _rtu_master[2];
rt_mq_t uart_mq = RT_NULL;

/* 用于放解析数据的内存池 */
static uint16_t temp_buff[128];

struct rt_semaphore g_tErrSem;


Modbus_Date ui_sample[MAX_SAMPLE_NUM];
Modbus_Date ui_cfgVal[MAX_CONFIG_NUM];
//TA设置项
Modbus_Date ui_taVal[MAX_TA_NUM];

uint8_t FirstBoot = 0;

#define TYPE_SAMPLE_DATA 22
#define TYPE_CFG_DATA 28
#define TYPE_TA_SAMPLE_DATA 29
#define TYPE_TA_CFG_DATA 	30


void modbus_assign_data(uint16_t type, uint8_t nums, uint16_t *rx_Data)
{
	uint16_t j = 0;
	Modbus_Date *tmp;
	switch(type)
	{
		case TYPE_SAMPLE_DATA:
			tmp = &ui_sample[0];
		break;
		case TYPE_CFG_DATA:
			tmp = &ui_cfgVal[0];
		break;
		case TYPE_TA_SAMPLE_DATA:
			tmp = &ui_taVal[0];
			j = TA_T_NUMS_END + 1;  //跳过保存
		break;
		case TYPE_TA_CFG_DATA:
			tmp = &ui_taVal[0];
		break;		
	}
	for(int i = 0; i < nums;)
	{		
		if(i % 2 == 0)
		{
			tmp[j].word.hi = rx_Data[i++];
		}else{
			tmp[j].word.low = rx_Data[i++];
			j++;				
		}
	}		
}



static void trans_modbus_poll_thread(void *param)
{
	int uwRet = 0;
	small_modbus_t *smb_master;
	M_UartMsgEvent r_msgCode;
	while (1)
	{
		uwRet = rt_mq_recv(uart_mq, &r_msgCode, sizeof(r_msgCode), RT_WAITING_FOREVER);
		if(RT_EOK == uwRet)
		{
			uint8_t type;
			if(r_msgCode.i_msgType <= E_Modbus_Read){
				smb_master = (small_modbus_t *)param;
			}else{
				smb_master = ((small_modbus_t *)param) + 1;
			}
			modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);

			switch(r_msgCode.i_msgType)
			{
				case E_Modbus_Read:
				{
					type = TYPE_SAMPLE_DATA;
					if(r_msgCode.i_addr >= MODBUS_LD_CFG_ADDR)
						type = TYPE_CFG_DATA;				
					uwRet = modbus_read_registers(smb_master, r_msgCode.i_addr, r_msgCode.i_regNum, temp_buff);
					if (uwRet >= MODBUS_OK)
					{										
						modbus_assign_data(type, r_msgCode.i_regNum, temp_buff);
						//配置类数据刷新
						if(type == TYPE_CFG_DATA)
						{
							rt_sem_release(&g_tErrSem);	
						}						
					}else{
						//配置类数据错误重发
						if(type == TYPE_CFG_DATA)
							Gui_SendMessge(uart_mq, MODBUS_LD_CFG_ADDR, MAX_CONFIG_NUM * 2, E_Modbus_Read, 0);					
					}
					break;
				}
				case E_Modbus_TA_Read:
				{
					type = TYPE_TA_CFG_DATA;
					if(r_msgCode.i_addr >= MODBUS_TA_SAMPLE_ADDR)
						type = TYPE_TA_SAMPLE_DATA; 						
					uwRet = modbus_read_registers(smb_master, r_msgCode.i_addr, r_msgCode.i_regNum, temp_buff);
					if (uwRet >= MODBUS_OK)
					{										
						modbus_assign_data(type, r_msgCode.i_regNum, temp_buff);
						//配置类数据刷新
						if(type == TYPE_TA_CFG_DATA)
						{
							rt_sem_release(&g_tErrSem);	
						}						
					}else{
						//配置类数据错误重发
						if(type == TYPE_TA_CFG_DATA)
							Gui_SendMessge(uart_mq, MODBUS_TA_CFG_ADDR, TA_T_NUMS_END * 2, E_Modbus_TA_Read, 0);
					}
					break;
				}				
				case E_Modbus_Write:
				case E_Modbus_TA_Write:
				{
					uint16_t send_buff[2] = {r_msgCode.w_data.word.hi, r_msgCode.w_data.word.low};				
					modbus_write_registers(smb_master, r_msgCode.i_addr, 2, send_buff);							
				}
				break;
			}

		}
		rt_thread_mdelay(100);		
	}	
}


/*****************************************
*485收发控制引脚回调函数
******************************************/ 
static int uart_rts(int on)
{
	if (on)
	{
		rt_pin_write(RS485_RTS_PIN, PIN_HIGH);
	}
	else
	{
		rt_pin_write(RS485_RTS_PIN, PIN_LOW);
	}
	return 0;
}


int modbus_Initial(small_modbus_t *smb_master, const char *uart_name, bool enable485)
{
	
	struct serial_configure serial_config;
	modbus_init(smb_master, MODBUS_CORE_RTU, modbus_port_rtdevice_create(uart_name));
	serial_config.baud_rate = BAUD_RATE_19200;
	serial_config.data_bits = DATA_BITS_8;
	serial_config.stop_bits = STOP_BITS_1;
	serial_config.bufsz = RT_MODBUS_SIZE;
	serial_config.parity = PARITY_NONE;
	//配置串口
	modbus_rtu_set_serial_config(smb_master, &serial_config);
	//配置485
	if(enable485 == true)
	{
		rt_pin_mode(RS485_RTS_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(RS485_RTS_PIN, PIN_LOW);
		modbus_rtu_set_serial_rts(smb_master, uart_rts);
	}	
	//设置串口接收方式
	modbus_rtu_set_oflag(smb_master, RT_DEVICE_FLAG_INT_RX);
	return modbus_connect(smb_master);
}



void trans_modbusInit(void)
{
	rt_thread_t tid = RT_NULL;
	int i;
	int ret = -1;
	/*****************************************
	*1. 初始化2个串口
	******************************************/	
	ret = modbus_Initial(&_rtu_master[0], LD_UART_NAME, false);
	if(ret != MODBUS_OK)
	{
		rt_kprintf("init modbus failed!\n");
		return;
	}
	ret = modbus_Initial(&_rtu_master[1], TA_UART_NAME, true);
	if(ret != MODBUS_OK)
	{
		rt_kprintf("init modbus failed!\n");
		return;
	}
	/*****************************************
	*2. 初始化消息队列
	******************************************/		
	uart_mq = rt_mq_create("uart_mq", sizeof(M_UartMsgEvent), 50, RT_IPC_FLAG_FIFO);
	if (uart_mq != RT_NULL)
		rt_kprintf("uart_mq create success\n");

	/*****************************************
	*3. 创建线程，接收消息并解析
	******************************************/		
	tid = rt_thread_create("MesgMonitor", trans_modbus_poll_thread, &_rtu_master[0], 1024, 10, 10);
	if (tid != RT_NULL)
		rt_thread_startup(tid);	
	/*****************************************
	*4. 初始化错误信号量
	******************************************/		
	ret = rt_sem_init(&g_tErrSem, "errSem", 0, RT_IPC_FLAG_FIFO);
	if(ret != RT_EOK)
	{
		rt_kprintf("init errSem failed!\n");
		return;
	}	
	/*****************************************
	*5. 清空数据池
	******************************************/
	for(i = 0; i< MAX_CONFIG_NUM; i++)
		ui_cfgVal[i].recvDate = 0;
	for(i = 0; i< MAX_SAMPLE_NUM; i++)
		ui_sample[i].recvDate = 0;
	for(i = 0; i< MAX_TA_NUM; i++)
		ui_taVal[i].recvDate = 0;

}



