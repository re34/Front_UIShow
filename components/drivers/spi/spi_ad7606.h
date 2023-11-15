#ifndef __SPI_AD7606_H
#define __SPI_AD7606_H


#ifdef RT_USING_AD7606

#define AD7606_SPI_BUS_NAME    	"spi1" // 对应硬件
#define AD7606_DEVICE_NAME     	"ad7606"
#define AD7606_CS_PIN     	   	GET_PIN(A, 4)
#define AD7606_CVA_PIN     	   	GET_PIN(C, 9)

#define AD7606_OS0_PIN			GET_PIN(H, 2)
#define AD7606_OS1_PIN			GET_PIN(A, 8)
#define AD7606_OS2_PIN			GET_PIN(D, 5)

#define BUSY_IRQ_PIN			GET_PIN(C, 8)

#define AD_CVA_PORT				GPIOC
#define AD_CVA_PIN				GPIO_PIN_9

#define	AD7606_CHN_NUM			1

struct _tagAdcChip
{
	struct rt_semaphore dmaCpt_Sem[2];
	uint8_t i_FinalCnt;
};

extern struct _tagAdcChip chipContent;

extern void adc_TaskInit(void);

#endif

#endif
