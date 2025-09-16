#ifndef __DRV_SOFTI2C_BQ769X0_H__
#define __DRV_SOFTI2C_BQ769X0_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "stm32f1xx_hal.h"

#include "bms_config.h"
#include "usart.h"

#define BQ769X0_I2C_ADDR	0x08

#define CRC_KEY 0x07

#define LOW_BYTE(Data)			(uint8_t)(0XFF & Data)
#define HIGH_BYTE(Data)			(uint8_t)(0XFF & (Data >> 8))

#define BQ769X0_DELAY(ms)		osDelay(ms)

#define BQ769X0_MALLOC(size)	pvPortMalloc(size)
#define BQ769X0_FREE(rmem)		vPortFree(rmem)			

/********************************* pin config **************************/
// wake up
#define BQ769X0_TS1_GPIOx		GPIOA
#define BQ769X0_TS1_Pin			GPIO_PIN_15

// ALERT
#define BQ769X0_ALERT_GPIOx		GPIOB
#define BQ769X0_ALERT_Pin		GPIO_PIN_12
#define BQ769X0_ALERT_EXIT_IRQ	EXTI15_10_IRQn
/***********************************************************************/


/********************************* cell and temp **********************/

// BQ76920	cell :3~5   温度:1
// BQ76930	cell :6~10  温度:2
// BQ76940	cell :9~15  温度:3
#define BQ769X0_CELL_MAX 	BMS_CELL_MAX	// 最大支持多少串
#define BQ769X0_TMEP_MAX	BMS_TEMP_MAX	// 最多几路温度

/****************************************************************************/




/********************************* BQ769X0 DEBUG **************************/


#define BQ769X0_DEBUG_LEVEL	1


#if (BQ769X0_DEBUG_LEVEL == 0)

#define BQ769X0_ERROR(...)		do{}while(0)
#define BQ769X0_WARNING(...)	do{}while(0)
#define BQ769X0_INFO(...)		do{}while(0)

#elif (BQ769X0_DEBUG_LEVEL == 1)

#define BQ769X0_ERROR(fmt, arg...)                              \
		do														\
        {                                                       \
            print_usart1("[BQ769X0 ERROR]");                  	\
            print_usart1(fmt"\r\n", ##arg);                     	\
        } while(0)
#define BQ769X0_WARNING(fmt, arg...)                            \
		do														\
        {                                                       \
            print_usart1("[BQ769X0 WARNING]");                 	\
            print_usart1(fmt"\r\n", ##arg);                      	\
        } while(0)
#define BQ769X0_INFO(fmt, arg...)                               \
		do														\
        {                                                       \
            print_usart1("[BQ769X0 INFO]");                  		\
            print_usart1(fmt"\r\n", ##arg);                      	\
        } while(0)


#elif (BQ769X0_DEBUG_LEVEL == 2)

#define BQ769X0_ERROR(fmt, arg...)                              \
		do														\
        {                                                       \
            print_usart1("[BQ769X0 ERROR][%s:%s:%d] ",            \
                    __FILE__, __FUNCTION__, __LINE__);          \
            print_usart1(fmt"\r\n", ##arg);                      	\
        } while(0)
#define BQ769X0_WARNING(fmt, arg...)                            \
		do														\
        {                                                       \
            print_usart1("[BQ769X0 WARNING][%s:%s:%d] ",          \
                    __FILE__, __FUNCTION__, __LINE__);          \
            print_usart1(fmt"\r\n", ##arg);                      	\
        } while(0)
#define BQ769X0_INFO(fmt, arg...)                               \
		do														\
        {                                                       \
            print_usart1("[BQ769X0 INFO][%s:%s:%d] ",             \
                    __FILE__, __FUNCTION__, __LINE__);          \
            print_usart1(fmt"\r\n", ##arg);						\
        } while(0)
        
#endif

		

/********************************************************************************/



#define SYS_STAT 				0x00
#define CELLBAL1 				0x01
#define CELLBAL2 				0x02
#define CELLBAL3 				0x03
#define SYS_CTRL1 				0x04
#define SYS_CTRL2 				0x05
#define PROTECT1 				0x06
#define PROTECT2 				0x07
#define PROTECT3 				0x08
#define OV_TRIP 				0x09
#define UV_TRIP 				0x0A
#define CC_CFG  				0x0B

#define VC1_HI_BYTE	 			0x0C
#define CC_HI_BYTE				0x32
#define CC_LO_BYTE				0x33
#define BAT_HI_BYTE				0x2A
#define TS1_HI_BYTE				0x2C
#define TS2_HI_BYTE				0x2E

#define ADCGAIN1 				0x50
#define ADCOFFSET 				0x51
#define ADCGAIN2 				0x59




#define SCD_THRESH_44mV_22mV	0x00
#define SCD_THRESH_67mV_33mV	0x01
#define SCD_THRESH_89mV_44mV	0x02
#define SCD_THRESH_111mV_56mV	0x03
#define SCD_THRESH_133mV_67mV	0x04
#define SCD_TRHESH_155mV_68mV	0x05
#define SCD_THRESH_178mV_89mV	0x06
#define SCD_THRESH_200mV_100mV	0x07

#define OCD_THRESH_17mV_8mV		0x00
#define OCD_THRESH_22mV_11mV	0x01
#define OCD_THRESH_28mV_14mV	0x02
#define OCD_THRESH_33mV_17mV	0x03
#define OCD_THRESH_39mV_19mV	0x04
#define OCD_THRESH_44mV_22mV	0x05
#define OCD_THRESH_50mV_25mV	0x06
#define OCD_THRESH_56mV_28MV	0x07
#define OCD_THRESH_61mV_31mV	0x08
#define OCD_THRESH_67mV_33mV	0x09
#define OCD_THRESH_72mV_36mV	0x0A
#define OCD_THRESH_78mV_39mV	0x0B
#define OCD_THRESH_83mV_42mV	0x0C
#define OCD_THRESH_89mV_44mV	0x0D
#define OCD_THRESH_94mV_47mV	0x0E
#define OCD_THRESH_100mV_50mV	0x0F

/*
#define SCD_DELAY_50us			0x00
#define SCD_DELAY_100us			0x01
#define SCD_DEALY_200us			0x02
#define SCD_DELAY_400us			0x03

#define OCD_DEALY_10ms			0x00
#define OCD_DELAY_20ms			0x01
#define OCD_DELAY_40ms			0x02
#define OCD_DELAY_80ms			0x03
#define OCD_DELAY_160ms			0x04
#define OCD_DELAY_320ms			0x05
#define OCD_DELAY_640ms			0x06
#define OCD_DELAY_1280ms		0x07

#define UV_DELAY_1s				0x00
#define UV_DELAY_4s				0x01
#define UV_DELAY_8s				0x02
#define UV_DELAY_16s			0x03

#define OV_DELAY_1s				0x00
#define OV_DELAY_2s				0x01
#define OV_DELAY_4s				0x02
#define OV_DELAY_8s				0x03
*/




#define OV_THRESH_BASE			0x2008
#define UV_THRESH_BASE			0x1000
#define OV_STEP					0x10
#define UV_STEP					0x10

#define ADCGAIN_BASE			365



#define SYS_STAT_OCD_BIT		0X01
#define SYS_STAT_SCD_BIT		0X02
#define SYS_STAT_OV_BIT			0X04
#define SYS_STAT_UV_BIT			0X08
#define SYS_STAT_OVRD_BIT		0X10
#define SYS_STAT_DEVICE_BIT		0X20
#define SYS_STAT_CC_BIT			0X80



typedef struct _Register_Group
{
	union
	{
		struct
		{
			uint8_t OCD				:1;
			uint8_t SCD				:1;
			uint8_t OV				:1;
			uint8_t UV				:1;
			uint8_t OVRD_ALERT		:1;
			uint8_t DEVICE_XREADY	:1;
			uint8_t WAKE			:1;
			uint8_t CC_READY		:1;
		}StatusBit;
		uint8_t StatusByte;
	}SysStatus;

	union
	{
		struct
		{
			uint8_t RSVD		:3;
			uint8_t CB5			:1;
			uint8_t CB4			:1;
			uint8_t CB3			:1;
			uint8_t CB2			:1;
			uint8_t CB1			:1;
		}CellBal1Bit;
		uint8_t CellBal1Byte;
	}CellBal1;

	union
	{
		struct
		{
			uint8_t RSVD		:3;
			uint8_t CB10		:1;
			uint8_t CB9			:1;
			uint8_t CB8			:1;
			uint8_t CB7			:1;
			uint8_t CB6			:1;
		}CellBal2Bit;
		uint8_t CellBal2Byte;
	}CellBal2;

	union
	{
		struct
		{
			uint8_t RSVD			:3;
			uint8_t CB15			:1;
			uint8_t CB14			:1;
			uint8_t CB13			:1;
			uint8_t CB12			:1;
			uint8_t CB11			:1;
		}CellBal3Bit;
		uint8_t CellBal3Byte;
	}CellBal3;

	union
	{
		struct
		{
			uint8_t SHUT_B			:1;
			uint8_t SHUT_A			:1;
			uint8_t RSVD1			:1;
			uint8_t TEMP_SEL		:1;
			uint8_t ADC_EN			:1;
			uint8_t RSVD2			:2;
			uint8_t LOAD_PRESENT	:1;
		}SysCtrl1Bit;
		uint8_t SysCtrl1Byte;
	}SysCtrl1;

	union
	{
		struct
		{
			uint8_t CHG_ON			:1;
			uint8_t DSG_ON			:1;
			uint8_t WAKE_T			:2;
			uint8_t WAKE_EN			:1;
			uint8_t CC_ONESHOT		:1;
			uint8_t CC_EN			:1;
			uint8_t DELAY_DIS		:1;
		}SysCtrl2Bit;
		uint8_t SysCtrl2Byte;
	}SysCtrl2;

	union
	{
		struct
		{
			uint8_t SCD_THRESH		:3;
			uint8_t SCD_DELAY		:2;
			uint8_t RSVD			:2;
			uint8_t RSNS			:1;
		}Protect1Bit;
		uint8_t Protect1Byte;
	}Protect1;

	union
	{
		struct
		{
			uint8_t OCD_THRESH		:4;
			uint8_t OCD_DELAY		:3;
			uint8_t RSVD			:1;
		}Protect2Bit;
		uint8_t Protect2Byte;
	}Protect2;

	union
	{
		struct
		{
			uint8_t RSVD			:4;
			uint8_t OV_DELAY		:2;
			uint8_t UV_DELAY		:2;
		}Protect3Bit;
		uint8_t Protect3Byte;
	}Protect3;

	uint8_t OVTrip;
	uint8_t UVTrip;
	uint8_t CCCfg;			//must be 0x19

	union
	{
		struct
		{
			uint8_t VC1_HI;
			uint8_t VC1_LO;
		}VCell1Byte;
		uint16_t VCell1Word;
	}VCell1;

	union
	{
		struct
		{
			uint8_t VC2_HI;
			uint8_t VC2_LO;
		}VCell2Byte;
		uint16_t VCell2Word;
	}VCell2;

	union
	{
		struct
		{
			uint8_t VC3_HI;
			uint8_t VC3_LO;
		}VCell3Byte;
		uint16_t VCell3Word;
	}VCell3;

	union
	{
		struct
		{
			uint8_t VC4_HI;
			uint8_t VC4_LO;
		}VCell4Byte;
		uint16_t VCell4Word;
	}VCell4;

	union
	{
		struct
		{
			uint8_t VC5_HI;
			uint8_t VC5_LO;
		}VCell5Byte;
		uint16_t VCell5Word;
	}VCell5;

	union
	{
		struct
		{
			uint8_t VC6_HI;
			uint8_t VC6_LO;
		}VCell6Byte;
		uint16_t VCell6Word;
	}VCell6;

	union
	{
		struct
		{
			uint8_t VC7_HI;
			uint8_t VC7_LO;
		}VCell7Byte;
		uint16_t VCell7Word;
	}VCell7;

	union
	{
		struct
		{
			uint8_t VC8_HI;
			uint8_t VC8_LO;
		}VCell8Byte;
		uint16_t VCell8Word;
	}VCell8;

	union
	{
		struct
		{
			uint8_t VC9_HI;
			uint8_t VC9_LO;
		}VCell9Byte;
		uint16_t VCell9Word;
	}VCell9;

	union
	{
		struct
		{
			uint8_t VC10_HI;
			uint8_t VC10_LO;
		}VCell10Byte;
		uint16_t VCell10Word;
	}VCell10;

	union
	{
		struct
		{
			uint8_t VC11_HI;
			uint8_t VC11_LO;
		}VCell11Byte;
		uint16_t VCell11Word;
	}VCell11;

	union
	{
		struct
		{
			uint8_t VC12_HI;
			uint8_t VC12_LO;
		}VCell12Byte;
		uint16_t VCell12Word;
	}VCell12;

	union
	{
		struct
		{
			uint8_t VC13_HI;
			uint8_t VC13_LO;
		}VCell13Byte;
		uint16_t VCell13Word;
	}VCell13;

	union
	{
		struct
		{
			uint8_t VC14_HI;
			uint8_t VC14_LO;
		}VCell14Byte;
		uint16_t VCell14Word;
	}VCell14;

	union
	{
		struct
		{
			uint8_t VC15_HI;
			uint8_t VC15_LO;
		}VCell15Byte;
		uint16_t VCell15Word;
	}VCell15;

	union
	{
		struct
		{
			uint8_t BAT_HI;
			uint8_t BAT_LO;
		}VBatByte;
		uint16_t VBatWord;
	}VBat;

	union
	{
		struct
		{
			uint8_t TS1_HI;
			uint8_t TS1_LO;
		}TS1Byte;
		uint16_t TS1Word;
	}TS1;

	union
	{
		struct
		{
			uint8_t TS2_HI;
			uint8_t TS2_LO;
		}TS2Byte;
		uint16_t TS2Word;
	}TS2;

	union
	{
		struct
		{
			uint8_t TS3_HI;
			uint8_t TS3_LO;
		}TS3Byte;
		uint16_t TS3Word;
	}TS3;

	union
	{
		struct
		{
			uint8_t CC_HI;
			uint8_t CC_LO;
		}CCByte;
		uint16_t CCWord;
	}CC;

	union
	{
		struct
		{
			uint8_t RSVD1			:2;
			uint8_t ADCGAIN_4_3		:2;
			uint8_t RSVD2			:4;
		}ADCGain1Bit;
		uint8_t ADCGain1Byte;
	}ADCGain1;

	uint8_t ADCOffset;

	union
	{
		struct
		{
			uint8_t RSVD			:5;
			uint8_t ADCGAIN_2_0		:3;
		}ADCGain2Bit;
		uint8_t ADCGain2Byte;
	}ADCGain2;

}RegisterGroup;





typedef enum
{
	BQ_SCD_DELAY_50us  = 0x00,
	BQ_SCD_DELAY_100us = 0x01,
	BQ_SCD_DELAY_200us = 0x02,
	BQ_SCD_DELAY_400us = 0x03,	
}BQ769X0_SCDDelayTypedef;

typedef enum
{
	BQ_OCD_DEALY_10ms	= 0x00,
	BQ_OCD_DELAY_20ms	= 0x01,
	BQ_OCD_DELAY_40ms	= 0x02,
	BQ_OCD_DELAY_80ms	= 0x03,
	BQ_OCD_DELAY_160ms	= 0x04,
	BQ_OCD_DELAY_320ms	= 0x05,
	BQ_OCD_DELAY_640ms	= 0x06,
	BQ_OCD_DELAY_1280ms	= 0x07,
}BQ769X0_OCDDelayTypedef;


typedef enum
{
	BQ_OV_DELAY_1s	= 0x00,
	BQ_OV_DELAY_2s	= 0x01,
	BQ_OV_DELAY_4s	= 0x02,
	BQ_OV_DELAY_8s	= 0x03,
}BQ769X0_OVDelayTypedef;

typedef enum
{
	BQ_UV_DELAY_1s	= 0x00,
	BQ_UV_DELAY_4s	= 0x01,
	BQ_UV_DELAY_8s	= 0x02,
	BQ_UV_DELAY_16s	= 0x03,
}BQ769X0_UVDelayTypedef;



typedef struct
{
	BQ769X0_SCDDelayTypedef SCDDelay;
	BQ769X0_OCDDelayTypedef OCDDelay;
	BQ769X0_OVDelayTypedef UVDelay;
	BQ769X0_UVDelayTypedef OVDelay;
	uint16_t UVPThreshold;
	uint16_t OVPThreshold;
}BQ769X0_ConfigDataTypedef;


// BQ硬件报警回调接口
typedef struct
{
	void (*ocd)(void);		// BQ769X0 充电过流硬件报警
	void (*scd)(void);		// BQ769X0 放电电路硬件报警
	void (*ov)(void);		// BQ769X0 充电过压硬件报警
	void (*uv)(void);		// BQ769X0 放电欠压硬件报警
	void (*ovrd)(void);		// BQ769X0 报警引脚由用户外围电路强行触发
	void (*device)(void);	// BQ769X0 设备故障报警
	void (*cc)(void);		// BQ769X0 库仑计采样完成
}BQ769X0_AlertOpsTypedf;

// 初始化数据结构体
typedef struct
{
	BQ769X0_AlertOpsTypedf AlertOps;
	BQ769X0_ConfigDataTypedef ConfigData;
}BQ769X0_InitDataTypedef;


typedef enum
{
	CHG_CONTROL = 0x01,
	DSG_CONTROL = 0x02
}BQ769X0_ControlTypedef;


typedef enum
{
	BQ_STATE_ENABLE,
	BQ_STATE_DISABLE
}BQ769X0_StateTypedef;


typedef enum
{
	BQ_CELL_INDEX1  = 0x0001,
	BQ_CELL_INDEX2  = 0x0002,
	BQ_CELL_INDEX3  = 0x0004,
	BQ_CELL_INDEX4  = 0x0008,
	BQ_CELL_INDEX5  = 0x0010,
	BQ_CELL_INDEX6  = 0x0020,
	BQ_CELL_INDEX7  = 0x0040,
	BQ_CELL_INDEX8  = 0x0080,
	BQ_CELL_INDEX9  = 0x0100,
	BQ_CELL_INDEX10 = 0x0200,
	BQ_CELL_INDEX11 = 0x0400,
	BQ_CELL_INDEX12 = 0x0800,
	BQ_CELL_INDEX13 = 0x1000,
	BQ_CELL_INDEX14 = 0x2000,
	BQ_CELL_INDEX15 = 0x4000,
	BQ_CELL_ALL		= 0x7FFF,
}BQ769X0_CellIndexTypedef;



typedef struct
{
	float CellVoltage[BQ769X0_CELL_MAX];	// 单节电芯电压
	float TsxTemperature[BQ769X0_TMEP_MAX];	// 热敏电阻温度
	float BatteryCurrent;	// 电池包总电流
	float BatteryVoltage;	// 电池包总电压
	float DieTemperature;	// ic温度,目前还未测试成功
}BQ769X0_SampleDataTypedef;



extern BQ769X0_SampleDataTypedef BQ769X0_SampleData;


void BQ769X0_Initialize(BQ769X0_InitDataTypedef *InitData);
void BQ769X0_Wakeup(void);
void BQ769X0_EntryShip(void);
bool BQ769X0_LoadDetect(void);
void BQ769X0_ControlDSGOrCHG(BQ769X0_ControlTypedef ControlType, BQ769X0_StateTypedef NewState);
void BQ769X0_CellBalanceControl(BQ769X0_CellIndexTypedef CellIndex, BQ769X0_StateTypedef NewState);

void BQ769X0_UpdateCellVolt(void);
void BQ769X0_UpdateTsTemp(void);
void BQ769X0_UpdateDieTemp(void);
void BQ769X0_UpdateCurrent(void);
void BQ769X0_UpadteBatVolt(void);

void BQ769X0_SCDDelaySet(BQ769X0_SCDDelayTypedef SCDDelay);
void BQ769X0_OCDDelaySet(BQ769X0_OCDDelayTypedef OCDDelay);
void BQ769X0_UVDelaySet(BQ769X0_UVDelayTypedef UVDelay);
void BQ769X0_OVDelaySet(BQ769X0_OVDelayTypedef OVDelay);
void BQ769X0_UVPThresholdSet(uint16_t UVPThreshold);
void BQ769X0_OVPThresholdSet(uint16_t OVPThreshold);







#endif


