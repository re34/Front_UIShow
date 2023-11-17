#ifndef __SPI_AD5541_H
#define __SPI_AD5541_H



#ifdef RT_USING_AD5541

#define AD5541_DEVICE_NAME  "ad5541"
#define AD5541_SPI_BUS_NAME "spi3"



#define AD5541_DATA_MIN                         0
#define AD5541_DATA_MAX       					65535  //0 A

#define _10Hz_HALF_PERIOD_TIME_US               50000         //(1/10hz)/2 = 0.05s 10hz半个周期的时长
#define SPI_DATA_SET_TIME_US              		42        	//ad5541发送间隔  42us 
#define _10Hz_TOTAL_STEPS_HALF_PERIOD           1000         //50000/42 = 1190.47  10hZ频率下半周期ad5541需要完成的描点数量

#define SAS_FIXED_SCAN_AMP                   22500
#define SAS_FIXED_SCAN_STEP                  30
#define MAX_STEP_NUM						 5

typedef enum
{
  State_In_ScanClose = 0,		//关闭扫描
  State_In_ScanOnly	 = 1,		//只进行扫描
  State_In_ScanNread_Half = 2,	//进行上沿或下沿扫描和采样
  State_In_ScanNread_Full = 3,  //进行全周期扫描和采样
} E_ScanState;

//搜索相位状态机
typedef enum
{
  State_In_dataSample = 0,		//扫描PZT上升沿并记录数据 
  State_In_dataAnalysis	 = 1,	//只进行扫描
} E_CfgPhaseState;

//自动锁状态机
typedef enum
{
  Lock_In_StartOn = 0,		//开始自动锁
  Lock_In_Sample = 0,		//自动锁采样阶段 
} E_AutoLockState;



#if defined(USING_HVPZT_SCAN)

typedef enum
{
  ChipID_HvScan_Amp = 0,		//幅值 （最大值-最小值）
  ChipID_HvScan_Bias = 1,	    //偏置
  ChipID_HvScan_DecBias = 2,	//扫描拉回偏置
} E_ChipID;



/* 由高压PZT幅值(mV)转换成芯片寄存器值 */
extern uint16_t HvFunc_Amp2RegVal(const long mvData);
/* 由高压PZT偏置值(mV)转换成芯片寄存器值 */
extern uint16_t HvFunc_Bias2RegVal(const long mvData);


#endif

extern uint8_t covNums;
extern struct rt_semaphore param_Sem;
extern struct rt_semaphore scanSw_Sem;

#endif

typedef  union _Dac_Para {
    uint32_t params[6];
    struct {
		uint32_t bias;			//扫描三角波中心偏置
		uint32_t scanAmplitude; 		//扫描三角波幅值
		uint32_t scanFreq;
		uint32_t scanState;		
		uint32_t chipID;
		uint32_t lockStat;		
    }chipVal;
}Dac_Para;




#endif

