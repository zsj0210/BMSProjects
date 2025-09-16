#ifndef __BMS_HAL_SAMPLE_H__
#define __BMS_HAL_SAMPLE_H__


#include <stdbool.h>

#include "bms_type.h"
#include "bms_config.h"


void Bms_HalMonitorCellVoltage(void);
void Bms_HalMonitorBatteryVoltage(void);
void Bms_HalMonitorBatteryCurrent(void);
void Bms_HalMonitorCellTemperature(void);
bool Bms_HalMonitorLoadDetect(void);


#endif

