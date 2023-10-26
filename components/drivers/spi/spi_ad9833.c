#include <rtthread.h>


#if defined(RT_USING_AD9833)

#include "application.h"
#include "drv_spi.h"
#include "mod_dac.h"
#include "spi_ad9833.h"

#define AD9833_SPI_BUS_NAME    "spi2" // 对应硬件
#define AD9833_DEVICE_NAME     "ad9833"
#define AD9833_CS_PIN     	   GET_PIN(B, 6)

extern MOD_INSERT g_tLaserManager;



/************************************************
* 1. 若驱动程序不使用rttread提供的驱动，只是用HAL库
* 2. 宏控制需要失能 BSP_USING_SPI2, 否则drv.spi.c 会使能spi2相关配置，
*    并调用stm32f4xx_hal_msp.c中的gpio初始化，相应的gpio初始化需要自己添加
*************************************************/

#ifdef RT_USING_RTOS_DRIVER

static int rt_hw_Ad9833_Config(void)
{
    rt_err_t res;
	struct stm32_hw_spi_cs *spi_cs;
	struct rt_spi_device *spi_dev_ad9833;

	spi_dev_ad9833 = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
	RT_ASSERT(spi_dev_ad9833 != RT_NULL);
	spi_cs = (struct stm32_hw_spi_cs *)rt_malloc(sizeof(struct stm32_hw_spi_cs));
	RT_ASSERT(spi_cs != RT_NULL);

	/* ad9833 use Pb6 as CS */
    spi_cs->GPIOx = GPIOB;
    spi_cs->GPIO_Pin = GPIO_PIN_6;

    rt_pin_mode(AD9833_CS_PIN, PIN_MODE_OUTPUT);    /* 设置片选管脚模式为输出 */
	rt_pin_write(AD9833_CS_PIN, PIN_HIGH);

    res = rt_spi_bus_attach_device(spi_dev_ad9833, AD9833_DEVICE_NAME, AD9833_SPI_BUS_NAME, (void*)spi_cs);
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

        rt_spi_configure(spi_dev_ad9833, &cfg);
    }
    return RT_EOK;
}


static int rt_hw_DacModule_init(void)
{
	rt_hw_Ad9833_Config();
	rt_kprintf("rt_hw_AD9833_init \n");    
	return 0;
}
INIT_COMPONENT_EXPORT(rt_hw_DacModule_init);
//INIT_PREV_EXPORT(rt_hw_DacModule_init);      /* 组件自动初始化 */

#else

#include "stm32f4xx_hal.h"

SPI_HandleTypeDef hspi2;

static M_GpioxInfo g_tDdsCsPinPool[2] =
{
	{GPIOB, GPIO_PIN_6},
	{GPIOB, GPIO_PIN_12},
};


static void MX_Spi2Bus_Init(M_GpioxInfo *CsPinPool)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitTypeDef CS_InitStruct = {0};

	/************************************************
	* spi gpio驱动使能
	*************************************************/
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
	
    /**SPI2 GPIO Configuration
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_16BIT;   //16位
	hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 10;
	HAL_SPI_Init(&hspi2);
}

#endif



static uint16_t HW_SelectPhaseRegister(E_PhaseRegSelect phase_reg, uint16_t data)
{
    data |= PHASE0_Address; /*写相位寄存器时，位 D15 和 D14 都置 1*/
    data = phase_reg < PHASE1 ? data & 0xDFFF : data | 0x2000;
    return data;
}


/******************************
 * @brief  ad983绘制目标波形
 * @note
 *******************************/
static void HW_Ad9833SetWave(M_DacChipOpr *pDacChip, uint8_t chipID)
{
	M_Ad9833Para *pData = (M_Ad9833Para *)pDacChip->userData;

    float fre_data = pData->frequency * AD9833_FREQ;
    uint16_t sfr_head = pData->freq_sfr ? FREQ1_Address : FREQ0_Address;
    uint16_t freq_lsb = ((uint32_t)fre_data & 0x3fff) | sfr_head;   
	//D15,和D14控制是Freq0 还是 Freq1
    uint16_t freq_msb = (((uint32_t)fre_data >> 14U) & 0x3fff) | sfr_head; 
	//D15和D14都是0, D13控制是phase0 还是 phase1
    uint16_t phase_data = HW_SelectPhaseRegister((E_PhaseRegSelect)pData->phase_sfr, pData->phase);
    uint16_t wave_mode = pData->wave_mode < Wave_Type_Tri ? AD9833_SIN_WAVE_CMD: pData->wave_mode < Wave_Type_Squ ? AD9833_TRI_WAVE_CMD: AD9833_SQU_WAVE_CMD;
	/*当AD9833初始化时，为了避免DAC产生虚假输出，
	 *RESET必须置为1（RESET不会复位频率、相位和控制寄存器），
	 *直到配置完毕，需要输出时才将RESET置为0 */
    rt_kprintf("sfr_head = 0x%04x, freq_lsb = 0x%04x, freq_msb = 0x%04x, phase_data = 0x%04x, wave_mode = 0x%04x\n", sfr_head, freq_lsb, freq_msb, phase_data, wave_mode);
    uint16_t _cmdTable[] = {
        AD9833_RESET_CMD,       /*复位ad9833*/
        AD9833_WRITE_ONECE_CMD, /*选择数据一次写入，b28位和reset位为1*/			
        freq_lsb,               // L14，选择频率寄存器0的低14位数据输入
        freq_msb,               // H14 频率寄存器的高14位数据输入
        phase_data,             // 设置相位
        wave_mode,              // 设置波形
    };
	/******************************
	* spi 硬件相关(用户添加)
	****************************/
#ifdef RT_USING_RTOS_DRIVER
	for (uint16_t *pCmd = _cmdTable; pCmd < _cmdTable + sizeof(_cmdTable) / sizeof(_cmdTable[0]); ++pCmd)
	{
		rt_spi_send(pDacChip->spiDevice, pCmd, sizeof(*pCmd) / sizeof(uint16_t));
	}
#else
	M_GpioxInfo *pCsPin = &pDacChip->tAllCsPin[chipID];
	for (uint16_t *pCmd = _cmdTable; pCmd < _cmdTable + sizeof(_cmdTable) / sizeof(_cmdTable[0]); ++pCmd)
	{
		HAL_GPIO_WritePin((GPIO_TypeDef* )pCsPin->pGPIOx, pCsPin->Gpio_Pin, (GPIO_PinState)GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi2, (uint8_t *)pCmd, sizeof(*pCmd) / sizeof(uint16_t), 1000);
		HAL_GPIO_WritePin((GPIO_TypeDef* )pCsPin->pGPIOx, pCsPin->Gpio_Pin, (GPIO_PinState)GPIO_PIN_SET);  
	}
#endif
}


M_DacChipOpr *HW_Ad9833Register(void *argv)
{
    M_DacChipOpr *mod_dds = NULL;

    mod_dds = (M_DacChipOpr *)rt_malloc(sizeof(M_DacChipOpr));
    if(mod_dds == NULL)
    {
        return NULL;
    }
	/*****************************************
	*一. private数据赋值, 由ini 文件传参进来
	******************************************/	
	mod_dds->userData = (M_Ad9833Para *)argv;
	if(mod_dds->userData == NULL)
    {
    	rt_free(mod_dds);
        return NULL;
    }
	/********************************************
	*1)使用操作系统获取句柄, 裸奔就直接引脚初始化
	*********************************************/	
#ifdef RT_USING_RTOS_DRIVER
	mod_dds->spiDevice = (struct rt_spi_device *)rt_device_find(AD9833_DEVICE_NAME);
	if(mod_dds->spiDevice == RT_NULL)
	{
		rt_kprintf("rt_device_find fail! ");
		rt_free(mod_dds);
		return NULL;
	}
#else
	mod_dds->tAllCsPin = &g_tDdsCsPinPool[0];
    MX_Spi2Bus_Init(mod_dds->tAllCsPin);

#endif
	/*********************
	*二. public数据赋值
	*********************/
    mod_dds->name = "dac_ad9833";
	mod_dds->_SetWaveFunc = HW_Ad9833SetWave;
	/*执行输出目标波形*/
	mod_dds->_SetWaveFunc(mod_dds, ((M_Ad9833Para *)argv)->chipID);
	/**********************
	*用户可添加线程相关代码
	**********************/
  	return mod_DacChipRegister(mod_dds);
}

/**
 * @brief   命令行：设置ad9833输出目标波形
 * @details
 * @param   none
 * @retval  none
 */
static void Msh_SetAd9833(int argc, char **argv)
{
	//直接获取正在运行的芯片
	M_DacChipOpr *pDacChip = GetMatchDacChip(g_tLaserManager.mod_dac, "dac_ad9833");
	if(pDacChip == NULL)
	{
		rt_kprintf("@error: no such device.\n");
		return;
	}
    if (argc != 7)
    {
        rt_kprintf("@error:params Error, please input'<Msh_SetAd9833 (freq)|(freq_sfr)|(wave: sin/tri/squ)|(phase)|(phase_sfr)|(Chipid)>.'\n");
        return;
    }
    float uFreq = (float)atof(argv[1]);
    if (uFreq > 12000000.0F)
    {
        rt_kprintf("@error:wrong frequency ,please input'0.1hz-12Mhz'.\n");
        return;
    }
    uint8_t iFreqInx = (float)atoi(argv[2]);
    if (iFreqInx > 1)
    {
        rt_kprintf("@error:wrong frequency register,please input'0/1'.\n");
        return;
    }	
    uint8_t iWave_type = (uint8_t)atoi(argv[3]);
    if (iWave_type > 3U)
    {
        rt_kprintf("@error:wrong wave type,please input'0:sin;1:tri;2:squ'.\n");
        return;
    }
    uint16_t wPhase = (uint16_t)atoi(argv[4]);
    if (wPhase > 360U)
    {
        rt_kprintf("@error:wrong phase,please input'0-360'.\n");
        return;
    }
    E_PhaseRegSelect iPhaseInx = (E_PhaseRegSelect)atoi(argv[5]);
    if (iPhaseInx > PHASE1)
    {
        rt_kprintf("@error:wrong phase register,please input'0/1'.\n");
        return;
    }		
    uint8_t chipID = (uint8_t)atoi(argv[6]);
    if (chipID > 1)
    {
        rt_kprintf("@error:wrong chipID,please input'0/1'.\n");
        return;
    }		
	M_Ad9833Para *pData = (M_Ad9833Para *)pDacChip->userData;
	pData->wave_mode = iWave_type;
	pData->phase = wPhase;
	pData->frequency = uFreq;
	pData->freq_sfr = iFreqInx;
	pData->phase_sfr = iPhaseInx;
	pData->chipID = chipID;

	pDacChip->_SetWaveFunc(pDacChip, chipID);
}
MSH_CMD_EXPORT(Msh_SetAd9833, operate sample: Msh_SetAd9833(freq) | (freq_sfr) |(wave sin / tri / squ) | (phase) | (phase_sfr) >);

#endif

