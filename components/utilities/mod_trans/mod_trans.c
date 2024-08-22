#include "mod_trans.h"
#include "small_modbus.h"
#include "board.h"

#include "lvgl.h"
#include "ui_utils.h"

#include "ulog.h"

//串口2    rts PA1
#define RS485_RTS_PIN    GET_PIN(A, 1)

#define TYPE_SAMPLE_DATA 22
#define TYPE_CFG_DATA 28
#define TYPE_TA_SAMPLE_DATA 29
#define TYPE_TA_CFG_DATA 	30

#define RT_SERIAL_CONFIG_USER           \
{                                          \
    BAUD_RATE_115200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_FIFO_BUFSZ, /* Buffer size */  \
    RT_SERIAL_FLOWCONTROL_NONE, /* Off flowcontrol */ \
    0                                      \
}


static small_modbus_t _rtu_master[2];

rt_mq_t uart_mq = RT_NULL;
rt_mq_t fiberMsg_mq = RT_NULL;

/* 用于放解析数据的内存池 */
static uint16_t temp_buff[128];
struct rt_semaphore g_tErrSem;


Modbus_Date ui_sample[MAX_SAMPLE_NUM];
Modbus_Date ui_cfgVal[MAX_CONFIG_NUM];
//TA设置项
Modbus_Date ui_taVal[MAX_TA_NUM];



struct _fiberParam ui_FiberConf;


//光纤激光器查询类
tagGetATRecall _pATCmd[_cmd_index_max] = {
	{{"?IMAXUSR\r\n"},	 {"IMAX_CW",""},	1},   
	{{"?IMAXMFG\r\n"},	 {"IMAX_CW",""},	1},
	{{"?STATUS\r\n"},	 {"STATUS",""},		-1}, 
	{{"?OPMD\r\n"},		 {"OPMOD",""},		1},
	{{"?EXPD\r\n"},		 {"EXPD",""},		-1},
	{{"?OUTPUT\r\n"},	 {"ISET",""},		1},
	{{"?DSN\r\n"},		 {"DSN",""},		1},	
	{{"ENABLE=\r\n"},	 {"ENABLED\r\n",""},	0}, 
	{{"OPMD=\r\n"},	 	 {"OPMD=",""},		0}, 
	{{"OUT=\r\n"},	 	 {"ISET=",""},		0}, 	
};

uint8_t recvbuf[256];
rt_device_t uart_dev;



/****************************hex ascii 码转换*************************************/
// 功能描述  : "9"的ASCII码与"A"的ASCII码值相差==7  注：字母都统一为大写字母的ASCII码值
uint8_t High_HexToAsc(uint8_t c1)
{
    c1 = c1 >> 4;
    return(c1 >= 0xa ? (c1 + 0x37) : (c1 + 0x30));
}

// 功能描述  : "9"的ASCII码与"A"的ASCII码值相差==7  注：字母都统一为大写字母的ASCII码值
uint8_t Low_HexToAsc(uint8_t c1)
{
    c1 = c1 & 0xf;
    return(c1 >= 0xa ? (c1 + 0x37) : (c1 + 0x30));
}

// 功能描述  : 把ascii 的'0'~'f' 转换成 0~f 一个byte
uint8_t AscToHex(uint8_t c1)
{
    if (c1 >= 0x30 && c1 <= 0x39) 
		c1 = c1 - 0x30;
    else if (c1 >= 0x41 && c1 <= 0x46)	
		c1 = c1 - 0x37;
    else if (c1 >= 0x61 && c1 <= 0x66)	
		c1 = c1 - 0x57;
    return(c1);
}


// 功能描述  : 把两个ASC码合成一个HEX字符
uint8_t TwoAscToHex(uint8_t TPA, uint8_t TPB)
{
    TPA = AscToHex(TPA);
    TPB = AscToHex(TPB);
    return((TPA << 4) | TPB);
}

// 功能描述  : 把ASC码缓冲转换成HEX缓冲区---uint32
int strToHex(uint8_t *numStr, uint16_t len)
{
    uint8_t  val, *pSrc, *ptr;
    uint16_t   i;
	int isOdd = 0;

	if(len == 0) return -1;
	if(len % 2 != 0)
	{
		isOdd = -1;
	    len = (len + 1) >> 1;	//奇数个数字符串前面补0
	}else
	    len >>= 1;
    ptr = pSrc = numStr;

    for(i = 0; i < len; i++)
	{
		if(i == 0 && isOdd < 0)	
		{
			val = 0;
		}else{
			val = *ptr++;
		}
        *pSrc++ = TwoAscToHex(val, *ptr++);
    }
	return 0;
}


/******************************************************
** 	截取字符串中某一段
*   输入：  	src         目标字符串
*           startsign   起始字符
*           endsign     结束字符
*           Num         起始第几个字符
*************************************************/
int GrepStrbyDot(const char src[],char startCode,char endCode, uint8_t sn, uint16_t *startIdx, uint16_t *endIdx)
{
    int retVal = -1;
    const char *ptr = src;
    uint8_t Cnt = 0;

    while (*ptr != '\0'){
        if (*ptr == startCode) {
            if (++Cnt == sn) {
                *startIdx = ptr - src + 1;
                retVal = *startIdx;
                ptr++;
                break;
            }
        }
        ptr++;
    }
    while (*ptr != '\r' && *ptr != endCode && *ptr != '\0'){
        ptr++;
    }
    *endIdx = ptr - src;
    return retVal;
}

int str2uint32(const char *str, uint32_t size, uint32_t *value)
{
    int index;
    uint32_t decValue = 0;
    
    if (size < 1 || size > 32) {
        return -1;
    } 
    for (index = 0; index < size;index++)
    {
        if ('0' <= str[index] && str[index] <= '9')
        {
           decValue = decValue * 10;
           decValue += str[index] - 0x30;
        }else{
            return -1;
        }
    }
    *value = decValue;    
    return 0;
}




int FIBER_ATcmd(uint8_t index, char *buf)
{  
	M_AtCmdMsgEvent m_Msg;
	rt_size_t len = 0;
	char *data_ptr = RT_NULL;
	
	if(buf == RT_NULL)
		data_ptr = (char *)_pATCmd[index].CmdBuff;
	else
		data_ptr = buf;
	
	len = rt_strlen((const char *)data_ptr);
	if(len > 0)
	{
		rt_device_write(uart_dev, 0, data_ptr, len);
		m_Msg.index = index;
		m_Msg.mode = 1;
		return rt_mq_send(fiberMsg_mq, &(m_Msg), sizeof(m_Msg)); 
	}
	return -1;
}


static void trans_FiberUart_thread(void *param)
{
	rt_size_t recv_sz;
	char *result; 
	int uwRet = 0;
	rt_device_t uart_dev = (rt_device_t)param;
	M_AtCmdMsgEvent r_msgCode;
	//接收到的字节流中有效数据的长度
	int rcv_len = 0, pos = 0; 
	uint16_t _startPos = 0, _endPos = 0;
	int i, parse_len;
	
	for(;;)
	{
		uwRet = rt_mq_recv(fiberMsg_mq, &r_msgCode, sizeof(r_msgCode), RT_WAITING_FOREVER);
		if(RT_EOK == uwRet)
		{
			if(r_msgCode.index >= _cmd_index_max) continue;
			while(1)
			{
				/*****************************************************
				*1. 阻塞读取，有数据就返回，否则阻塞并切换运行其他线程
				******************************************************/
				recv_sz = rt_device_read(uart_dev, -1, &recvbuf[rcv_len], 128 - rcv_len);
				if (recv_sz > 0) 
				{
					if (rcv_len == 0) 
					{
						/**********************************************
						*1. 第一帧先从字节流中找到帧头
						***********************************************/
						result = rt_strstr((const char *)recvbuf,(const char *)_pATCmd[r_msgCode.index].CmdReceBuff[0]);
						if(result != RT_NULL)
						{
							pos = result - (char *)recvbuf;
							if(pos == 0)
							{
								rcv_len = recv_sz;
							}
							else if (pos < recv_sz)
							{	
								//拷贝有效段
								rcv_len = recv_sz - pos;
								rt_memcpy(recvbuf, &recvbuf[pos], recv_sz - pos);
							}	
						}else {
							rcv_len = 0;
							continue;
						}
					} else {
						rcv_len += recv_sz;
					}
					/*****************************
					* 2. 数据不足或 数据帧尾不是以回车结束，则继续接收
					*****************************/
					if (rcv_len < 2 || (recvbuf[rcv_len - 2] != '\r' && recvbuf[rcv_len - 1] != '\n')) 
					{	
						continue;
					}
					/*****************************
					* 3. 解析接收完的字符串
					*****************************/	
					switch(r_msgCode.index)
					{
						case _cmd_index_get_status:
							GrepStrbyDot((const char *)recvbuf, '=', '\r', 1, &_startPos, &_endPos);	
							uwRet = strToHex(recvbuf + _startPos, _endPos - _startPos);
							if(!uwRet)
							{
								for(i = 0; i < 8; i++)
								{
									ui_FiberConf.warn_val[i] = recvbuf[_startPos + i];
								}
							}														
						break;
						case _cmd_index_get_id:
							
						break;
						case _cmd_index_get_mode:	
							GrepStrbyDot((const char *)recvbuf, '=', '\r', 1, &_startPos, &_endPos);
							recvbuf[_endPos] = '\0';

							if(!rt_strcmp("DGC", (const char *)recvbuf + _startPos))
								ui_FiberConf.mode = fiber_mode_DGC;
							else if(!rt_strcmp("DGQ", (const char *)recvbuf + _startPos))
								ui_FiberConf.mode = fiber_mode_DGQ;
							else if(!rt_strcmp("ANG", (const char *)recvbuf + _startPos))
								ui_FiberConf.mode = fiber_mode_ANG;	
						break;						
						case _cmd_index_get_PD:
							parse_len = 0;
							//解析出PD1~4
							for(i = 0; i < 4; i++)
							{
								if(i == 0)
									GrepStrbyDot((const char *)recvbuf + parse_len, '=', ',', 1, &_startPos, &_endPos);
								else
									GrepStrbyDot((const char *)recvbuf + parse_len, ',', ',', 1, &_startPos, &_endPos);
								uwRet = strToHex(recvbuf + _startPos + parse_len, _endPos - _startPos);
								if(!uwRet){
									ui_FiberConf.Pd_val[i] = ((uint16_t)(recvbuf[_startPos + parse_len] << 8)) | recvbuf[_startPos + parse_len + 1];
									rt_kprintf("Pd_val[%d]=%d\n", i, ui_FiberConf.Pd_val[i]);
								}
								parse_len += _endPos;
							}
						break;
						case _cmd_index_get_I_lv1:					
						case _cmd_index_get_I_lv2:
							GrepStrbyDot((const char *)recvbuf, '=', 'R', 1, &_startPos, &_endPos);
							uwRet = strToHex(recvbuf + _startPos, _endPos - _startPos);
							if(!uwRet){
								ui_FiberConf.I_UserMax[r_msgCode.index] = ((uint16_t)(recvbuf[_startPos] << 8)) | recvbuf[_startPos + 1];
								rt_kprintf("I_UserMax[%d]=%d\n", r_msgCode.index, ui_FiberConf.I_UserMax[r_msgCode.index]);
							}
						break;
						//获取输出参数（电流、频率、占空比）
						case _cmd_index_get_output:
							if(ui_FiberConf.mode == fiber_mode_DGQ)
							{
								parse_len = 0;
								for(i = 0; i < _cfg_index_Max; i++)
								{
									GrepStrbyDot((const char *)recvbuf + parse_len, '=', ',', 1, &_startPos, &_endPos);
									uwRet = strToHex(recvbuf + _startPos + parse_len, _endPos - _startPos);
									if(!uwRet){
										if(i == _cfg_index_Freq)
											ui_FiberConf.cfg_val[i] = ((uint32_t)(recvbuf[_startPos + parse_len] << 16)) | ((uint16_t)(recvbuf[_startPos + parse_len + 1] << 8)) | recvbuf[_startPos + parse_len + 2];	
										else
											ui_FiberConf.cfg_val[i] = ((uint16_t)(recvbuf[_startPos + parse_len] << 8)) | recvbuf[_startPos + parse_len + 1];
										rt_kprintf("cfg_val[%d]=%d\n", i, ui_FiberConf.cfg_val[i]);
									}
									parse_len += _endPos;
								}
							}else if(ui_FiberConf.mode == fiber_mode_DGC){
								GrepStrbyDot((const char *)recvbuf, '=', '\r', 1, &_startPos, &_endPos);
								uwRet = strToHex(recvbuf + _startPos, _endPos - _startPos);
								if(!uwRet){
									ui_FiberConf.cfg_val[_cfg_index_ISet] = ((uint16_t)(recvbuf[_startPos] << 8)) | recvbuf[_startPos + 1];
								}
							}			
						break;
						//设置命令不需要解析回复
						case _cmd_index_set_switch:	
						case _cmd_index_set_mode:
						case _cmd_index_set_output:
						break;
					}
					//无论结果如何都要通知查询下一个
					if(r_msgCode.index < _cmd_index_get_id)
						rt_sem_release(&g_tErrSem);
					rcv_len = 0;
					break;
				}
			}
		}

	}
}


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
			//modbus_error_recovery(smb_master);
			modbus_set_slave(smb_master, 1);
	
			switch(r_msgCode.i_msgType)
			{
				case E_Modbus_Read:
				{
					type = TYPE_SAMPLE_DATA;
					if(r_msgCode.i_addr >= MODBUS_DFB_CFG_ADDR)
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
							Gui_SendMessge(uart_mq, MODBUS_DFB_CFG_ADDR, MAX_CONFIG_NUM * 2, E_Modbus_Read, 0);					
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


int modbus_Initial(small_modbus_t *smb_master,  const char *uart_name, bool enable485)
{
	
	struct serial_configure serial_config;
	//init modbus
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
	modbus_rtu_set_oflag(smb_master, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
	return modbus_connect(smb_master);
}


int fiberUart_init(void)
{
    struct serial_configure uart_conf = RT_SERIAL_CONFIG_USER;

    uart_dev = rt_device_find(FIBER_UART_NAME);
    if (uart_dev == RT_NULL) {
        rt_kprintf("Find device: %s failed\n", FIBER_UART_NAME);
        return -1;
    }
    rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &uart_conf);
	//使用dma及空闲中断
	if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", FIBER_UART_NAME);
        return -1;
    }
	return 0;
}



void trans_modbusInit(void)
{
	rt_thread_t Dfb_tid = RT_NULL, Fiber_tid = RT_NULL;

	
	int i;
	int ret = -1;
	/*****************************************
	*1. 初始化2个串口: DFB(modbus); FIBER(atcmd)
	******************************************/	
	ret = modbus_Initial(&_rtu_master[0], DFB_UART_NAME, false);
	if(ret != MODBUS_OK)
	{
		rt_kprintf("init %s modbus failed!\n", DFB_UART_NAME);
		return;
	}
	ret = fiberUart_init();
	if(ret < 0)
	{
		rt_kprintf("init fiber %s failed!\n", FIBER_UART_NAME);
		return;
	}
	
	/*****************************************
	*2. 初始化消息队列
	******************************************/		
	uart_mq = rt_mq_create("uart_mq", sizeof(M_UartMsgEvent), 50, RT_IPC_FLAG_FIFO);
	if (uart_mq != RT_NULL)
		rt_kprintf("uart_mq create success\n");
	
	fiberMsg_mq = rt_mq_create("fiberMsg_mq", sizeof(M_UartMsgEvent), 50, RT_IPC_FLAG_FIFO);
	if (fiberMsg_mq != RT_NULL)
		rt_kprintf("fiberMsg_mq create success\n");

	/*****************************************
	*3. 创建线程，接收消息并解析
	******************************************/		
	Dfb_tid = rt_thread_create("MesgMonitor", trans_modbus_poll_thread, (void *)&_rtu_master[0], 1024, 10, 10);
	if (Dfb_tid != RT_NULL)
		rt_thread_startup(Dfb_tid);
	
	Fiber_tid = rt_thread_create("AtCmdMonitor", trans_FiberUart_thread, (void *)uart_dev, 1024, 10, 10);
	if (Fiber_tid != RT_NULL)
		rt_thread_startup(Fiber_tid);		
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



