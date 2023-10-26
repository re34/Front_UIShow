#include <rtthread.h>


#if defined(RT_USING_AD7606)

#include "application.h"
#include "drv_spi.h"
#include "spi_ad7606.h"



extern MOD_INSERT g_tLaserManager;
struct rt_semaphore adc_Sem;


struct rt_spi_device *spiDev;

uint16_t adc_readPool[8];


#ifdef BSP_SPI1_RX_USING_DMA
struct rt_semaphore dmaCpt_Sem;

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	rt_sem_release(&dmaCpt_Sem);

}
#endif

static int rt_hw_Ad7606_Config(void)
{
    rt_err_t res;
	struct stm32_hw_spi_cs *spi_cs;
	struct rt_spi_device *spi_dev_ad7606;

	spi_dev_ad7606 = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
	RT_ASSERT(spi_dev_ad7606 != RT_NULL);
	spi_cs = (struct stm32_hw_spi_cs *)rt_malloc(sizeof(struct stm32_hw_spi_cs));
	RT_ASSERT(spi_cs != RT_NULL);


    rt_pin_mode(AD7606_CVA_PIN, PIN_MODE_OUTPUT);    /* 设置启动转换管脚模式为输出 */
	rt_pin_write(AD7606_CVA_PIN, PIN_LOW);

    rt_pin_mode(AD7606_OS0_PIN, PIN_MODE_OUTPUT);    /* 设置OS0为输出 0*/
	rt_pin_write(AD7606_OS0_PIN, PIN_LOW);

    rt_pin_mode(AD7606_OS1_PIN, PIN_MODE_OUTPUT);    /* 设置OS1为输出 0*/
	rt_pin_write(AD7606_OS1_PIN, PIN_LOW);

    rt_pin_mode(AD7606_OS2_PIN, PIN_MODE_OUTPUT);    /* 设置OS2为输出 0*/
	rt_pin_write(AD7606_OS2_PIN, PIN_LOW);

	/* ad7606 use PA4 as CS */
    spi_cs->GPIOx = GPIOA;
    spi_cs->GPIO_Pin = GPIO_PIN_4;
    rt_pin_mode(AD7606_CS_PIN, PIN_MODE_OUTPUT);    /* 设置片选管脚模式为输出 */
	rt_pin_write(AD7606_CS_PIN, PIN_HIGH);
#ifdef BSP_SPI1_RX_USING_DMA
	rt_sem_init(&dmaCpt_Sem, "dmaSem", 0, RT_IPC_FLAG_PRIO);
	spi_dev_ad7606->user_data = (void *)&dmaCpt_Sem;
#endif
    res = rt_spi_bus_attach_device(spi_dev_ad7606, AD7606_DEVICE_NAME, AD7606_SPI_BUS_NAME, (void*)spi_cs);
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
        cfg.max_hz = 2 * 1000 *1000; /* stm32_spi_init 分频系数 16*/
        rt_spi_configure(spi_dev_ad7606, &cfg);
    }
    return RT_EOK;
}


static int rt_hw_AdcModule_init(void)
{
	rt_hw_Ad7606_Config();
	rt_kprintf("rt_hw_ad7606_init \n");    
	return 0;
}
INIT_COMPONENT_EXPORT(rt_hw_AdcModule_init);


static void Msh_Ad7606ConvertStart(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	//空闲状态下
	if(PIN_LOW == rt_pin_read(BUSY_IRQ_PIN))
	{	
		//暂停pwm固定周期adc采样
		//__HAL_TIM_DISABLE(&htim3);
		//GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, DISABLE);	
		__HAL_RCC_GPIOC_CLK_ENABLE();	
		/**********************************
		*	PC9 ------> CovertA_B
		************************************/
		GPIO_InitStruct.Pin = AD_CVA_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(AD_CVA_PORT, &GPIO_InitStruct);
		
		HAL_GPIO_WritePin(AD_CVA_PORT, AD_CVA_PIN, GPIO_PIN_RESET);
		rt_thread_delay(RT_TICK_PER_SECOND / 1000);
		HAL_GPIO_WritePin(AD_CVA_PORT, AD_CVA_PIN, GPIO_PIN_SET);
	}else
		rt_kprintf("ad7606 chips is busying \n");
}

MSH_CMD_EXPORT(Msh_Ad7606ConvertStart, ad7606 Convert Trigger Once);

static void Busy_Occur_callback(void *args)
{	 
	rt_sem_release(&adc_Sem);
}


void adc_read_thread(void *param)
{	
	int i;
	while(1)
	{
		//busy 信号空闲
		rt_sem_take(&adc_Sem, RT_WAITING_FOREVER);
		for (i = 0; i < 8; i++)
		{
			rt_spi_recv(spiDev, &adc_readPool[i], 1);
		}
		//rt_kprintf("adc_read[%d] = 0x%04x\n", i, adc_readPool[0]);
	}
}

void adc_TaskInit(void)
{
	rt_thread_t tid;
	
	spiDev = (struct rt_spi_device *)rt_device_find(AD7606_DEVICE_NAME);
	if(spiDev == RT_NULL)
	{
		rt_kprintf("rt_device_find adc fail! ");
		return;
	}	
	int ret = rt_sem_init(&adc_Sem, "adcSem", 0, RT_IPC_FLAG_PRIO);
	if (ret != RT_EOK)
	{
		rt_kprintf("init adcSem failed!\n");
		return;
	}
	tid = rt_thread_create("adcRead", adc_read_thread, RT_NULL, 512, 20, 10);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	/*****************************************
	*  初始化中断脚 BUSY引脚
	*******************************************/
	rt_pin_mode(BUSY_IRQ_PIN, PIN_MODE_INPUT);	
	rt_pin_attach_irq(BUSY_IRQ_PIN, PIN_IRQ_MODE_FALLING, Busy_Occur_callback, RT_NULL);
	rt_pin_irq_enable(BUSY_IRQ_PIN, PIN_IRQ_ENABLE);		
}

#endif
