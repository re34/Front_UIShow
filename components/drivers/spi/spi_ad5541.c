#include <rtthread.h>

#ifdef RT_USING_AD5541
#include "mod_dac.h"


#define AD5541_DEVICE_NAME  "ad5541"


SPI_HandleTypeDef hspi3;

static M_GpioxInfo g_tDacCsPinPool[1] =
{
	{GPIOD, GPIO_PIN_2},
};

static void MX_Spi3Bus_Init(M_GpioxInfo *CsPinPool)
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
	for(int i = 0; i < 2; i++)
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
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	HAL_SPI_Init(&hspi3);
}



//tick = rt_tick_get();
//while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2)) ;

static void HW_Ad5541SetWave(M_DacChipOpr *pDacChip, uint8_t chipID)
{
	



}


void HW_AD5541Write(long pzt_set_to)
{
	HAL_GPIO_WritePin((GPIO_TypeDef* )pCsPin->pGPIOx, pCsPin->Gpio_Pin, (GPIO_PinState)GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, (uint8_t *)pCmd, sizeof(*pCmd) / sizeof(uint16_t), 1000);
	HAL_GPIO_WritePin((GPIO_TypeDef* )pCsPin->pGPIOx, pCsPin->Gpio_Pin, (GPIO_PinState)GPIO_PIN_SET);  
}



void Task_ScanNReadProc(void* parameter)
{
	M_Ad5541Para *s_Para = (M_Ad5541Para *)parameter;
    long tmpVal = 0;//中间变量
    static long val_min = 0;	
    static long val_max = 0;
	static uint16_t StepLen = 0;

	for(;;)
	{
		//挂起线程，接收开始信号
		uwRet = rt_mq_recv(uart_mq,							//一直等待获取消息
							&uMsgCode,
							sizeof(uMsgCode),
							RT_WAITING_FOREVER
							);		
		while(1)
		{
			//无阻塞接受信号量, 接受完修改参数或直接停止扫描
			if(RT_EOK == rt_sem_trytake(hrate_ready))
			{
				rt_kprintf("thread1: recv msg from msg queue, the content:%c\n", buf);
				if(!s_Para->scanAmplitude! = State_In_ScanNread)
				{
					break;
				}else{
					//描点范围0~65535（2^16 - 1）
					val_min = s_Para->centerOffSet - s_Para->scanAmplitude;	
					val_max = s_Para->centerOffSet + s_Para->scanAmplitude;
					if(val_min < AD5541_DATA_MIN)
					{
						val_min = AD5541_DATA_MIN;
					}		
					if(val_max > AD5541_DATA_MAX)
					{
						val_max = AD5541_DATA_MAX;
					}
					StepLen = s_Para->pointYStep;
				}
			}	
			for(tmpVal = val_min; tmpVal <= val_max; tmpVal += StepLen)
			{
				HW_AD5541Write(tmpVal);
				rt_sem_release(rt_sem_t sem); //释放信号给ad7606去采集，同时切换cs脚
			}
			//下降沿不进行采集处理 快速执行 (*data_ad5541_pzt_scan_step_ratio_downward的原因）
	        for(tmpVal = val_max; tmpVal >= val_min; tmpVal -= (StepLen * 3)
	        {
	            HW_AD5541Write(tmpVal);
	        }  			
			rt_thread_mdelay(1);
		}
	}
}





void Task_ScanOnlyProc(void* parameter)
{
	M_Ad5541Para *s_Para = (M_Ad5541Para *)parameter;
    long tmpVal = 0;
    static long val_min = 0;	
    static long val_max = 0;
	static uint16_t y_AmpStepLen = 0;
	static uint16_t x_TimerStepLen = 0;
	long HalfTimeUs = 0;
	long TotalSteps = 0;
	
	for(;;)
	{
		//挂起线程，接收开始信号
		uwRet = rt_mq_recv(uart_mq,							//一直等待获取消息
							&uMsgCode,
							sizeof(uMsgCode),
							RT_WAITING_FOREVER
							);		
		while(1)
		{
			//无阻塞接受信号量, 接受完修改参数或直接停止扫描
			if(RT_EOK == rt_sem_trytake(hrate_ready))
			{
				rt_kprintf("thread1: recv msg from msg queue, the content:%c\n", buf);
				if(s_Para->scanAmplitude! = State_In_ScanOnly)
				{
					break;
				}else{
					//描点范围0~65535（2^16 - 1）
					val_min = s_Para->centerOffSet - s_Para->scanAmplitude;	
					val_max = s_Para->centerOffSet + s_Para->scanAmplitude;
					if(val_min < AD5541_DATA_MIN)
					{
						val_min = AD5541_DATA_MIN;
					}		
					if(val_max > AD5541_DATA_MAX)
					{
						val_max = AD5541_DATA_MAX;
					}
    				HalfTimeUs = 1000000 / s_Para->scanFreq / 2;    //1/f /2  * 10^6 (转化成us)
    				//计算描点步进长度， 确定y坐标
					TotalSteps = _10Hz_TOTAL_STEPS_HALF_PERIOD * HalfTimeUs /_10Hz_HALF_PERIOD_TIME_US; 					
			        y_AmpStepLen = (val_max - val_min) / TotalSteps + 1;	
					s_Para->pointYStep = y_AmpStepLen;
					//计算描点时间长度， 确定x坐标
					x_TimerStepLen = HalfTimeUs / TotalSteps - SPI_DATA_SET_TIME_US;
					s_Para->pointXStep = x_TimerStepLen;
				}
			}	
			for(tmpVal = val_min; tmpVal <= val_max; tmpVal += y_AmpStepLen)
			{
				HW_AD5541Write(tmpVal);
				rt_hw_us_delay(x_TimerStepLen);
			}
	        for(tmpVal = val_max; tmpVal >= val_min; tmpVal -= y_AmpStepLen)
	        {
	            HW_AD5541Write(tmpVal);
				rt_hw_us_delay(x_TimerStepLen);
	        }
			rt_thread_mdelay(1);
		}
	}

}




M_DacChipOpr *HW_Ad5541Register(void *argv)
{
    M_DacChipOpr *mod_dac = NULL;
	rt_thread_t tid_ScanNread = RT_NULL;
	rt_thread_t tid_ScanOnly = RT_NULL;
	
    mod_dac = (M_DacChipOpr *)rt_malloc(sizeof(M_DacChipOpr));
    if(mod_dac == NULL)
    {
        return NULL;
    }
	mod_dac->userData = (M_Ad5541Para *)argv;
	if(mod_dac->userData == NULL)
    {
    	rt_free(mod_dac);
        return NULL;
    }
	/*********************
	*private数据赋默认值
	*********************/
#ifdef RT_USING_RTOS_DRIVER
	mod_dac->spiDevice = (struct rt_spi_device *)rt_device_find(AD5541_DEVICE_NAME);
	if(rt_spi_device != RT_EOK )
	{
		rt_kprintf("rt_device_find fail! ");
		rt_free(mod_dac);
		return NULL;
	}
#else
	mod_dac->tAllCsPin = &g_tDacCsPinPool[0];
	MX_Spi3Bus_Init(mod_dac->tAllCsPin);
#endif
	/*********************
	*public数据赋值
	*********************/
    mod_dac->name = "dac_ad5541";
	mod_dac->_SetWaveFunc = HW_Ad5541SetWave;
	/*执行输出目标波形*/
	mod_dac->_SetWaveFunc(mod_dac, ((M_Ad5541Para *)argv)->chipID);
	/**********************
	*用户添加线程相关代码
	**********************/
	tid_ScanNread = rt_thread_create("scanNread_proc", Task_ScanNReadProc, (void *)mod_dac->userData, 512, 3, 20);
	if (tid_ScanNread != NULL)
		rt_thread_startup(tid_ScanNread);
	else
		return -1;
	
	tid_ScanNread = rt_thread_create("scanOnly_proc", Task_ScanOnlyProc, (void *)mod_dac->userData, 512, 3, 20);
	if (tid_ScanNread != NULL)
		rt_thread_startup(tid_ScanNread);
	else
		return -1;

    return mod_DacChipRegister(mod_dac);
}

#endif
