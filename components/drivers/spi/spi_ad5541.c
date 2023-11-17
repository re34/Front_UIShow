#include <rtthread.h>



#include "application.h"
#include "drv_spi.h"
#include "mod_dac.h"
#include "spi_ad5541.h"
#include "spi_ad7606.h"

struct rt_semaphore param_Sem;
struct rt_semaphore scanSw_Sem;

uint8_t covNums = 0;

const char *paramName[4] = {
	"bias",		
	"amp",
	"freq",
	"stat",	
};


typedef struct
{
    long u32Amp;
    long u32Step;
}ScanParam_t;





#ifdef RT_SPI3_USING_RTOS_DRIVER

#define AD5541_CS_PIN     	   GET_PIN(D, 2)

static int rt_hw_Ad5541_Config(void)
{
    rt_err_t res;
	struct stm32_hw_spi_cs *spi_cs;
	struct rt_spi_device *spi_dev_ad5541;

	spi_dev_ad5541 = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
	RT_ASSERT(spi_dev_ad5541 != RT_NULL);
	spi_cs = (struct stm32_hw_spi_cs *)rt_malloc(sizeof(struct stm32_hw_spi_cs));
	RT_ASSERT(spi_cs != RT_NULL);

	/* ad5541 use PD2 as CS */
    spi_cs->GPIOx = GPIOD;
    spi_cs->GPIO_Pin = GPIO_PIN_2;

    rt_pin_mode(AD5541_CS_PIN, PIN_MODE_OUTPUT);    /* 设置片选管脚模式为输出 */
	rt_pin_write(AD5541_CS_PIN, PIN_HIGH);

    res = rt_spi_bus_attach_device(spi_dev_ad5541, AD5541_DEVICE_NAME, AD5541_SPI_BUS_NAME, (void*)spi_cs);
    if (res != RT_EOK)
    {
        rt_kprintf("attach Fail!\r\n");
        return res;
    }
    /* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 16;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_2 | RT_SPI_MSB;
        cfg.max_hz = 3 * 1000 *1000; /* stm32_spi_init 分频系数 16*/

        rt_spi_configure(spi_dev_ad5541, &cfg);
    }
    return RT_EOK;
}

static int rt_hw_Dac5541_init(void)
{
	rt_hw_Ad5541_Config();
	rt_kprintf("rt_hw_AD5541_init \n");    
	return 0;
}
INIT_COMPONENT_EXPORT(rt_hw_Dac5541_init);

#else

SPI_HandleTypeDef hspi3;


#if defined(USING_HVPZT_SCAN)
/*
static M_GpioxInfo g_tDacCsPinPool[] =
{
	{GPIOD, GPIO_PIN_2},		//HV_amp
	{GPIOD, GPIO_PIN_3}, 		//HV_bias
	{GPIOD, GPIO_PIN_7},		//HV_scan_bias  (拉回偏置)
};
*/
static M_GpioxInfo g_tDacCsPinPool[] =
{
	{GPIOC, GPIO_PIN_13},		//HV_amp   cs1---DAC_SCAN
	{GPIOB, GPIO_PIN_9}, 		//HV_bias  cs2---DAC_BIAS
	{GPIOD, GPIO_PIN_3},		//HV_scan_bias  (拉回偏置) cs3---DAC_SCAN_BIAS
};

#else
static M_GpioxInfo g_tDacCsPinPool[] =
{
	{GPIOD, GPIO_PIN_2},
};	
#endif
static void MX_Spi3Bus_Init(M_GpioxInfo *CsPinPool, uint8_t csNum)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitTypeDef CS_InitStruct = {0};

	/************************************************
	* spi gpio驱动使能
	*************************************************/
    __HAL_RCC_SPI3_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	//初始化所有从机cs脚
	/*****************************************
	*待解决问题： 如何连带使能对应的gpio时钟
	*****************************************/
	CS_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	CS_InitStruct.Pull = GPIO_PULLUP;
	CS_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	for(int i = 0; i < csNum; i++)
	{
	    CS_InitStruct.Pin = CsPinPool[i].Gpio_Pin;
	    HAL_GPIO_Init((GPIO_TypeDef *)CsPinPool[i].pGPIOx, &CS_InitStruct);
		HAL_GPIO_WritePin((GPIO_TypeDef *)CsPinPool[i].pGPIOx, CsPinPool[i].Gpio_Pin, GPIO_PIN_SET);
	}
	/* SPI3 parameter configuration*/
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_16BIT;   //16位
	hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	HAL_SPI_Init(&hspi3);
}

#endif

static void HW_AD5541Write(M_DacChipOpr *pDacChip, uint8_t chipID, uint16_t data)
{
#ifdef RT_SPI3_USING_RTOS_DRIVER
	rt_spi_send(pDacChip->spiDevice, data, 1);
#else
	M_GpioxInfo *pCsPin = &pDacChip->tAllCsPin[chipID];	
	HAL_GPIO_WritePin((GPIO_TypeDef *)pCsPin->pGPIOx, pCsPin->Gpio_Pin, (GPIO_PinState)GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, (uint8_t *)&data, 1, 1000);
	HAL_GPIO_WritePin((GPIO_TypeDef* )pCsPin->pGPIOx, pCsPin->Gpio_Pin, (GPIO_PinState)GPIO_PIN_SET); 
#endif
}

static void HW_Ad5541SetWave(M_DacChipOpr *pDacChip, uint8_t chipID, uint16_t data)
{
	HW_AD5541Write(pDacChip, chipID, data);
}






#if defined(USING_HVPZT_SCAN)

/* 由高压PZT幅值(mV)转换成芯片寄存器值 */
uint16_t HvFunc_Amp2RegVal(const long mvData)
{
    return (uint16_t)(mvData / 50000.0 * 65535);
}


/* 由高压PZT偏置值(mV)转换成芯片寄存器值 */
uint16_t HvFunc_Bias2RegVal(const long mvData)
{
    return (uint16_t)(mvData / 135000.0 * 65535);
}

/******************************************************
* 计算原理：确定一个点的坐标（x,y）
*  x: 与时间/频率相关，决定了多久描一个点，得到点数（10HZ 1200个点）
*  y: = 幅值 / 点数
*******************************************************/
void Task_ScanOrReadProc(void* parameter)
{
	rt_err_t uwRet = RT_ERROR;
	Dac_Para *s_Para = (Dac_Para *)parameter;
	static uint16_t yStepLen = 0;
	//寄存器值
	uint32_t i32PztRegBias = 0;
	uint32_t i32PztRegAmp = 0;
	
	M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");

	for(;;)
	{
		//挂起线程，接收串口信号 或任务信号(执行1次)  
		uwRet = rt_sem_take(&scanSw_Sem, RT_WAITING_FOREVER);
		if(RT_EOK == uwRet && s_Para->chipVal.scanState)
		{
			i32PztRegBias = HvFunc_Bias2RegVal(s_Para->chipVal.bias);
			i32PztRegAmp = HvFunc_Amp2RegVal(s_Para->chipVal.scanAmplitude);
			HW_AD5541Write(pDacChip, ChipID_HvScan_Bias, i32PztRegBias);
			HW_AD5541Write(pDacChip, ChipID_HvScan_DecBias, (i32PztRegAmp + 1) / 2);
			HW_AD5541Write(pDacChip, ChipID_HvScan_Amp, 0);
			yStepLen = i32PztRegAmp * s_Para->chipVal.scanFreq / 10000 + 1;
			while(1)
			{
				if(RT_EOK == rt_sem_trytake(&param_Sem)) //参数发生变化
				{
					if(s_Para->chipVal.scanState == State_In_ScanClose)
					{
						//退出当前循环
						break;
					}
					i32PztRegBias = HvFunc_Bias2RegVal(s_Para->chipVal.bias);
					i32PztRegAmp = HvFunc_Amp2RegVal(s_Para->chipVal.scanAmplitude);
					HW_AD5541Write(pDacChip, ChipID_HvScan_Bias, i32PztRegBias);
					HW_AD5541Write(pDacChip, ChipID_HvScan_DecBias, (i32PztRegAmp + 1) / 2);
					HW_AD5541Write(pDacChip, ChipID_HvScan_Amp, 0);
					//计算步进值 = 幅值/ 需要描点的数量
					yStepLen = i32PztRegAmp * s_Para->chipVal.scanFreq / 10000 + 1;
				}
				int32_t tmpval;
			    for ( tmpval = 0; tmpval <= i32PztRegAmp; tmpval += yStepLen)
			    {
			        HW_AD5541Write(pDacChip, ChipID_HvScan_Amp, tmpval);
			        rt_hw_us_delay(42);
			    }
			    for(tmpval = i32PztRegAmp; tmpval >= 0; tmpval -= yStepLen)
			    {
					HW_AD5541Write(pDacChip, ChipID_HvScan_Amp, tmpval);         
			       	rt_hw_us_delay(42);
			    }				
			}
		}
	}
}

#else

void Task_ScanOrReadProc(void* parameter)
{
	rt_err_t uwRet = RT_ERROR;
	Dac_Para *s_Para = (Dac_Para *)parameter;
    long tmpVal = 0;//中间变量
    static long val_min = 0, val_max = 0;
	static uint16_t yStepLen = 0;
	M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");

	for(;;)
	{
		//挂起线程，接收串口信号 或任务信号(执行1次)  	
		uwRet = rt_sem_take(&scanSw_Sem, RT_WAITING_FOREVER);
		if(RT_EOK == uwRet && s_Para->chipVal.scanState)
		{
			//初始化值
			switch(s_Para->chipVal.scanState)
			{
				case State_In_ScanNread_Half:
					s_Para->chipVal.scanAmplitude = SAS_FIXED_SCAN_AMP;
					yStepLen = SAS_FIXED_SCAN_STEP;		
				break;
				case State_In_ScanNread_Full:
					s_Para->chipVal.scanAmplitude = _stPztScanParams[s_Para->chipVal.lockStat].u32Amp;
					yStepLen = _stPztScanParams[s_Para->chipVal.lockStat].u32Step; 				
				break;
				default:
				break;				
			}			
			val_min = s_Para->chipVal.bias - s_Para->chipVal.scanAmplitude;	
		    val_max = s_Para->chipVal.bias + s_Para->chipVal.scanAmplitude;
			if(s_Para->chipVal.scanState == State_In_ScanOnly)
				yStepLen = (val_max - val_min) * s_Para->chipVal.scanFreq / 10000 + 1;			
			while(1)
			{
				//无阻塞接受信号量,  三角波参数是否被更改
				if(RT_EOK == rt_sem_trytake(&param_Sem))
				{
					if(s_Para->chipVal.scanState == State_In_ScanClose)
					{
						//退出当前循环
						break;
					}
					if(s_Para->chipVal.scanState == State_In_ScanNread_Full)
					{
						s_Para->chipVal.scanAmplitude = _stPztScanParams[s_Para->chipVal.lockStat].u32Amp;
						yStepLen = _stPztScanParams[s_Para->chipVal.lockStat].u32Step; 
					}					
					val_min = s_Para->chipVal.bias - s_Para->chipVal.scanAmplitude;	
					val_max = s_Para->chipVal.bias + s_Para->chipVal.scanAmplitude;
					//描点范围0~65535（2^16 - 1）
					if(val_min < AD5541_DATA_MIN)
					{
						val_min = AD5541_DATA_MIN;
					}		
					if(val_max > AD5541_DATA_MAX)
					{
						val_max = AD5541_DATA_MAX;
					}
					//计算描点步进长度， 确定y坐标
					if(s_Para->chipVal.scanState == State_In_ScanOnly)
						yStepLen = (val_max - val_min) * s_Para->chipVal.scanFreq / 10000 + 1;			
				}
				//判断是否执行边描边采			
				if(s_Para->chipVal.scanState > State_In_ScanOnly)			
				{
					for(tmpVal = val_min; tmpVal <= val_max; tmpVal += yStepLen)
					{
						HW_AD5541Write(pDacChip, 0, tmpVal);					
						//让ad7606去采集(启动cov脚)，同时阻塞等待采集完毕			
						HAL_GPIO_WritePin(AD_CVA_PORT, AD_CVA_PIN, GPIO_PIN_RESET);
						rt_hw_us_delay(10);
						HAL_GPIO_WritePin(AD_CVA_PORT, AD_CVA_PIN, GPIO_PIN_SET);
						rt_sem_take(&chipContent.dmaCpt_Sem[1], RT_WAITING_FOREVER);
					}
					//下降沿不进行采集处理 快速执行 (*data_ad5541_pzt_scan_step_ratio_downward的原因）
			        for(tmpVal = val_max; tmpVal >= val_min; tmpVal -= (yStepLen * 3))
			        {
			            HW_AD5541Write(pDacChip, 0, tmpVal);
			        }
					break;
				}else if(s_Para->chipVal.scanState == State_In_ScanOnly){
					for(tmpVal = val_min; tmpVal <= val_max; tmpVal += yStepLen)
					{
						HW_AD5541Write(pDacChip, 0, tmpVal);
						rt_hw_us_delay(41);
					}
					for(tmpVal = val_max; tmpVal >= val_min; tmpVal -= yStepLen)
					{
						HW_AD5541Write(pDacChip, 0, tmpVal);
						rt_hw_us_delay(41);
					}
				}
				rt_thread_mdelay(1);
			}
		}
	}
}

#endif



M_DacChipOpr *HW_Ad5541Register(void *argv)
{
    M_DacChipOpr *mod_dac = NULL;
	rt_thread_t tid_Collect = RT_NULL;
	
    mod_dac = (M_DacChipOpr *)rt_malloc(sizeof(M_DacChipOpr));
    if(mod_dac == NULL)
    {
        return NULL;
    }
	mod_dac->userData = (Dac_Para *)argv;
	if(mod_dac->userData == NULL)
	{
		rt_free(mod_dac);
		return NULL;
	}
	/*********************
	*private数据赋默认值
	*********************/
#ifdef RT_SPI3_USING_RTOS_DRIVER
	mod_dac->spiDevice = (struct rt_spi_device *)rt_device_find(AD5541_DEVICE_NAME);
	if(rt_spi_device != RT_EOK )
	{
		rt_kprintf("rt_device_find fail! ");
		rt_free(mod_dac);
		return NULL;
	}
#else
	mod_dac->tAllCsPin = &g_tDacCsPinPool[0];
	MX_Spi3Bus_Init(mod_dac->tAllCsPin, sizeof(g_tDacCsPinPool) / sizeof(g_tDacCsPinPool[0]));
#endif
	/*********************
	*public数据赋值
	*********************/
	mod_dac->name = "dac_ad5541";
	mod_dac->_SetWaveFunc = HW_Ad5541SetWave;
	
	int ret = rt_sem_init(&scanSw_Sem, "scanSem", 0, RT_IPC_FLAG_FIFO);
	if(ret != RT_EOK)
	{
		rt_kprintf("init scanSw_Sem failed!\n");
		return NULL;
	}
	ret = rt_sem_init(&param_Sem, "paramSem", 0, RT_IPC_FLAG_FIFO);
	if(ret != RT_EOK)
	{
		rt_kprintf("init paramSem failed!\n");
		return NULL;
	}
	tid_Collect = rt_thread_create("monitor", Task_ScanOrReadProc, mod_dac->userData, 1024, 20, 10);
	if (tid_Collect != RT_NULL)
		rt_thread_startup(tid_Collect);
	return mod_DacChipRegister(mod_dac);
}




/**
 * @brief   命令行：设置ad5541输出指定电压
 * @details
 * @param   none
 * @retval  none
 */
static void Msh_SetAd5541(int argc, char **argv)
{
	//直接获取正在运行的芯片
	M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");
	if(pDacChip == NULL)
	{
		rt_kprintf("@error: no such device.\n");
		return;
	}
    if (argc != 3)
    {
        rt_kprintf("@error:params Error, please input'<Msh_SetAd5541 (setval)|(Chipid)>.\n");
        return;
    }
    uint16_t setval = (uint16_t)atoi(argv[1]);		
    uint8_t chipID = (uint8_t)atoi(argv[2]);
    if (chipID > 1)
    {
        rt_kprintf("@error:wrong chipID,please input'0/1'.\n");
        return;
    }		
	HW_AD5541Write(pDacChip, chipID, setval);
}
MSH_CMD_EXPORT(Msh_SetAd5541, operate sample: Msh_SetAd5541 (setval)|(Chipid) >);

/**
 * @brief   命令行：开启扫描模式1~3
 * @details
 * @param   none
 * @retval  none
 */
static void test_uartCmd(int argc, char **argv)
{
	if (argc != 2)
	{
		return;
	}else{
		M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");
		Dac_Para *s_Para =  (Dac_Para *)pDacChip->userData; 

		uint16_t tmp = atoi(argv[1]);
		if(s_Para->chipVal.scanState != tmp &&  tmp<= State_In_ScanNread_Full)
		{
			s_Para->chipVal.scanState = tmp;
			rt_sem_release(&scanSw_Sem);	
		}	
	}
}
MSH_CMD_EXPORT(test_uartCmd, operate sample: test_uartCmd<(scanState)>);


/*****************************************
 * @brief   命令行：关闭扫描或修改指定参数
 * @details
 * @param   none
 * @retval  none
 ******************************************/
static void test_paramChange(int argc, char **argv)
{
	if (argc != 3)
	{
		return;
	}else{
		int i = 0;
		M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad5541");
		Dac_Para *sPara =  (Dac_Para *)pDacChip->userData; 
		for(; i < 4; i++)
		{
			if(!rt_strcmp(paramName[i], argv[1]))
			{
				sPara->params[i] = atoi(argv[2]);
				break;
			}
		}
		//发送参数已改变的消息
		if(i < 4)
			rt_sem_release(&param_Sem);
	}
}
MSH_CMD_EXPORT(test_paramChange, operate sample: test_paramChange<(cmd)|(value)>);



