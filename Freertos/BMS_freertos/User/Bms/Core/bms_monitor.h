#ifndef __BMS_MONITOR_H__
#define __BMS_MONITOR_H__


#include <stdbool.h>

#include "bms_type.h"


typedef struct
{
	float CellVoltage; 		// 电芯电压
	uint32_t CellNumber;	// 电芯的编号
}BMS_CellDataTypedef;


typedef struct
{
	float CellTemp[BMS_TEMP_MAX];					// 采样温度,温度数据会从小到大排序
	float BatteryVoltage;							// 电池总电压
	float BatteryCurrent;							// 电池组电流
	BMS_CellDataTypedef CellData[BMS_CELL_MAX]; 	// 电芯数据,电压数据会从小到大排序
	float CellVoltage[BMS_CELL_MAX]; 				// 电芯电压,未排序的
	uint32_t CellTempEffectiveNumber;				// 有效值的温度数量
}BMS_MonitorDataTypedef;


extern BMS_MonitorDataTypedef BMS_MonitorData;


void BMS_MonitorInit(void);
void BMS_MonitorStateCellVoltage(BMS_StateTypedef NewState);
void BMS_MonitorStateBatVoltage(BMS_StateTypedef NewState);
void BMS_MonitorStateBatCurrent(BMS_StateTypedef NewState);
void BMS_MonitorStateCellTemp(BMS_StateTypedef NewState);

void BMS_MonitorHwCurrent(void);


#endif


