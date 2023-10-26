#ifndef __SPI_AD5541_H
#define __SPI_AD5541_H



#ifdef RT_USING_AD5541


#define AD5541_DATA_MIN                         0
#define AD5541_DATA_MAX       					65535  //0 A

#define _10Hz_HALF_PERIOD_TIME_US               50000         //(1/10hz)/2 = 0.05s 10hz半个周期的时长
#define SPI_DATA_SET_TIME_US              		42        	//ad5541发送间隔  42us 
#define _10Hz_TOTAL_STEPS_HALF_PERIOD           1000         //50000/42 = 1190.47  10hZ频率下半周期ad5541需要完成的描点数量





typedef enum
{
  State_In_ScanNread,
  State_In_ScanOnly,
} E_ScanState;

typedef struct
{
	uint8_t scanState;
	uint16_t centerOffSet;		    //扫描三角波中心偏置
	uint16_t scanAmplitude;		    //扫描三角波幅值
	uint16_t pointXStep;
	uint16_t pointYStep;
	uint16_t scanFreq;

} M_Ad5541Para;

#endif

#endif

