#ifndef __BMS_HAL_CONFIG_H__
#define __BMS_HAL_CONFIG_H__


#include <stdint.h>



typedef enum
{
	BMS_SCD_DELAY_50us  = 0x00,
	BMS_SCD_DELAY_100us = 0x01,
	BMS_SCD_DELAY_200us = 0x02,
	BMS_SCD_DELAY_400us = 0x03,	
}BMS_SCDDelayTypedef;


typedef enum
{
	BMS_OCD_DEALY_10ms	 = 0x00,
	BMS_OCD_DELAY_20ms	 = 0x01,
	BMS_OCD_DELAY_40ms	 = 0x02,
	BMS_OCD_DELAY_80ms	 = 0x03,
	BMS_OCD_DELAY_160ms	 = 0x04,
	BMS_OCD_DELAY_320ms	 = 0x05,
	BMS_OCD_DELAY_640ms	 = 0x06,
	BMS_OCD_DELAY_1280ms = 0x07,
}BMS_OCDDelayTypedef;


typedef enum
{
	BMS_OV_DELAY_1s	 = 0x00,
	BMS_OV_DELAY_2s	 = 0x01,
	BMS_OV_DELAY_4s	 = 0x02,
	BMS_OV_DELAY_8s  = 0x03,
}BMS_OVDelayTypedef;


typedef enum
{
	BMS_UV_DELAY_1s	 = 0x00,
	BMS_UV_DELAY_4s	 = 0x01,
	BMS_UV_DELAY_8s	 = 0x02,
	BMS_UV_DELAY_16s = 0x03,
}BMS_UVDelayTypedef;


void BMS_HalCfgSCDDelaySet(BMS_SCDDelayTypedef SCDDelay);
void BMS_HalCfgOCDDelaySet(BMS_OCDDelayTypedef OCDDelay);
void BMS_HalCfgUVDelaySet(BMS_UVDelayTypedef UVDelay);
void BMS_HalCfgOVDelaySet(BMS_OVDelayTypedef OVDelay);
void BMS_HalCfgUVPThresholdSet(uint16_t UVPThreshold);
void BMS_HalCfgOVPThresholdSet(uint16_t OVPThreshold);



#endif

