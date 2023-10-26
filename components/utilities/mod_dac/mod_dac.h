#ifndef __MOD_DAC_H
#define __MOD_DAC_H

#ifdef RT_USING_RTOS_DRIVER
#include <rtthread.h>
#endif


typedef struct _GpioxInfo
{
	void *pGPIOx;
	unsigned short Gpio_Pin;
} M_GpioxInfo;

typedef struct _DacChipOpr
{
	char *name; 	  
	void (*_SetWaveFunc)(struct _DacChipOpr *, unsigned char);			  
#ifdef RT_USING_RTOS_DRIVER
	struct rt_spi_device *spiDevice;
#else
	M_GpioxInfo *tAllCsPin;
#endif
	void *userData;	
	struct _DacChipOpr *ptNext;
} M_DacChipOpr;



#ifdef RT_USING_AD9833
	extern M_DacChipOpr *HW_Ad9833Register(void *argv);
#endif

#ifdef RT_USING_AD5541
	extern M_DacChipOpr *HW_Ad5541Register(void *argv);
#endif

extern M_DacChipOpr *mod_DacChipRegister(M_DacChipOpr *ptDacChipOpr);
extern M_DacChipOpr *GetMatchDacChip(M_DacChipOpr *head, char *pcName);
extern M_DacChipOpr* DacChipInit(void *argv);
extern M_DacChipOpr* DacChipExit(M_DacChipOpr **pHead, char *pcName);
#endif

