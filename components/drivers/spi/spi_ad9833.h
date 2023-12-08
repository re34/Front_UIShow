
#ifndef __SPI_AD9833_H
#define __SPI_AD9833_H



#ifdef RT_USING_AD9833

/*ad9833的28bit最大计数值:pow(2,28) /25 MHZ(mclk)*/
#define AD9833_FREQ             (10.7374182f)
/*复位ad9833,即reset位为1*/
#define AD9833_RESET_CMD        0x0100
#define AD9833_WRITE_ONECE_CMD  0x2100

#define Default_Freq_Channel	FREQ0
#define Default_Phase_Channel   PHASE0

#define FREQ0_Address 0x4000  //频率寄存器0的地址(D15=0    , D14=1)
#define FREQ1_Address 0x8000  //频率寄存器1的地址(D15=1    , D14=0)
#define PHASE0_Address 0xC000 //相位寄存器0的地址(D15=1, D14=1, D13 = 0)
#define PHASE1_Address 0xE000 //相位寄存器1的地址(D15=1, D14=1, D13 = 1)

/*输出正弦波命令*/
#define AD9833_SIN_WAVE_CMD 0x2000
/*输出三角波命令*/
#define AD9833_TRI_WAVE_CMD 0x2002
/*输出方波命令*/
#define AD9833_SQU_WAVE_CMD 0x2028


/* 定义AD9833频率寄存器枚举 */
typedef enum
{
  FREQ0,
  FREQ1
} E_FreqRegSelect;


/* 定义AD9833相位寄存器枚举 */
typedef enum Ad9833Phase
{
  PHASE0 = 0,
  PHASE1 = 1,
} E_PhaseRegSelect;



typedef enum
{
	Wave_Type_Sin = 0,
	Wave_Type_Tri,		
	Wave_Type_Squ,
	Wave_Type_Null = 0xFF,
} E_WaveTypeSelect;

#endif


typedef struct
{
	uint32_t frequency;		    //频率值
	uint8_t freq_sfr;			//频率寄存器编号
	uint16_t phase;			    //相位值
	uint8_t phase_sfr;			//相位寄存器编号
	uint8_t wave_mode;			//波形类型
	uint8_t chipID;				//片选信号编号
} M_Ad9833Para;




#endif

