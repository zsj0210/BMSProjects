#ifndef __BMS_ENERGY_H__
#define __BMS_ENERGY_H__


#include <rtthread.h>
#include "cmsis_os2.h"
#include "bms_type.h"


typedef struct
{
	float SocStopChg;			// 停止充电SOC值
	float SocStartChg;			// 启动充电SOC值
	float SocStopDsg;			// 停止放电SOC值
	float SocStartDsg;			// 启动放电SOC值
	
	float BalanceStartVoltage;	// 均衡起始电压(V)
	float BalanceDiffeVoltage;	// 均衡差异电压(V)
	uint32_t BalanceCycleTime;	// 均衡周期时间(s)
	BMS_CellIndexTypedef BalanceRecord;	// 均衡记录,正在均衡的会被位与上
	
}BMS_EnergyDataTypedef;


extern osSemaphoreId_t BalanceSem;
extern BMS_EnergyDataTypedef BMS_EnergyData;


void BMS_EnergyInit(void);



#endif

