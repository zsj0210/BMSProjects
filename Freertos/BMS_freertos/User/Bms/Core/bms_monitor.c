#define BMS_DBG_TAG "Monitor"

#include <stdio.h>
#include <rtthread.h>

#include "bms_monitor.h"

#include "bms_hal_monitor.h"

#include "bms_energy.h"
#include "bms_global.h"
#include "bms_debug.h"





// thread config
#define MONITOR_TASK_STACK_SIZE	512
#define MONITOR_TASK_PRIORITY	9
#define MONITOR_TASK_TIMESLICE	25

#define MONITOR_TASK_PERIOD		250



// sample time config  MS
#define UPDATE_CELL_VOLTAGE_CYCLE	250
#define UPDAYE_BAT_VOLTAGE_CYCLE	250
#define UPDATE_CELL_TEMP_CYCLE		2000
#define UPDATE_BAT_CURRENT_CYCLE	1000






BMS_MonitorDataTypedef BMS_MonitorData;



static bool FlagSampleIntCur = false;

static bool FlagCellVoltage = true;
static bool FlagBatVoltage = true;
static bool FlagCellTemp = true;
static bool FlagBatCurrent = true;


static uint16_t CountCellVoltage = 0;
static uint16_t CountBatVoltage = 0;
static uint16_t CountCellTemp = 0;
//static uint16_t CountBatCurrent = 0;


const osThreadAttr_t monitorTask_attributes = {
  .name = "monitorTask",
  .stack_size = MONITOR_TASK_STACK_SIZE,
  .priority = (osPriority_t) osPriorityNormal5,
};

static void BMS_MonitorTaskEntry(void *paramter);
static void BMS_MonitorBattery(void);
static void BMS_MonitorSysMode(void);


void BMS_MonitorInit(void)
{
	osThreadId_t thread;
	
	thread = osThreadNew(BMS_MonitorTaskEntry, NULL, &monitorTask_attributes);

   if (thread == NULL)
   {
	   BMS_ERROR("Create Task Fail");
   }
}

static void BMS_MonitorTaskEntry(void *paramter)
{
	while (1)
	{
		BMS_MonitorBattery();
		BMS_MonitorSysMode();
		osDelay(MONITOR_TASK_PERIOD);
	}
}


// 监控电池各项数据
static void BMS_MonitorBattery(void)
{
	// 单体电芯电压
	CountCellVoltage += MONITOR_TASK_PERIOD;
	if (FlagCellVoltage == true && CountCellVoltage >= UPDATE_CELL_VOLTAGE_CYCLE)
	{
		Bms_HalMonitorCellVoltage();
		CountCellVoltage = 0;
	}
	else if (FlagCellVoltage == false)
	{
		CountCellVoltage = 0;
	}
	
	// 电池组电压
	CountBatVoltage += MONITOR_TASK_PERIOD;
	if (FlagBatVoltage == true && CountBatVoltage >= UPDAYE_BAT_VOLTAGE_CYCLE)
	{
		Bms_HalMonitorBatteryVoltage();
		CountBatVoltage = 0;		
	}
	else if (FlagBatVoltage == false)
	{
		CountBatVoltage = 0;
	}

	// 电池温度
	CountCellTemp += MONITOR_TASK_PERIOD;
	if (FlagCellTemp == true && CountCellTemp++ >= UPDATE_CELL_TEMP_CYCLE)
	{
		Bms_HalMonitorCellTemperature();
		CountCellTemp = 0;
	}
	else if (FlagCellTemp == false)
	{
		CountCellTemp = 0;
	}


	/* 电流采样由软件触发 
	CountBatCurrent += MONITOR_TASK_PERIOD;
	if (FlagBatCurrent == true && CountBatCurrent >= UPDATE_BAT_CURRENT_CYCLE)
	{
		CountBatCurrent = 0;
		Bms_HalMonitorBatteryCurrent();
	}
	else if (FlagBatCurrent == false)
	{
		CountBatCurrent = 0;
	}
	*/


	/* 电流采样由硬件中断触发,太麻烦了,每次烧写都得重新给BQ重新下上电 */
	if (FlagSampleIntCur == true && FlagBatCurrent == true)
	{
		Bms_HalMonitorBatteryCurrent();
		FlagSampleIntCur = false;		
	}
}



// 系统模式监控
// BatteryCurrent > 20mA || BatteryCurrent < -20mA  处于非睡眠模式
// BatteryCurrent < 20mA || BatteryCurrent > -20mA  处于待机模式或者睡眠模式
// BatteryCurrent <= -20mA 处于放电模式
// BatteryCurrent >=  20mA 处于充电模式
static void BMS_MonitorSysMode(void)
{
	static BMS_SysModeTypedef SysModeBackup = BMS_MODE_NULL;
	static uint32_t StandbyCount = 0;
	
	if (BMS_GlobalParam.SysMode == BMS_MODE_SLEEP)
	{
		if ((BMS_MonitorData.BatteryCurrent >= 0.02) || (BMS_MonitorData.BatteryCurrent <= -0.02))
		{
			// 可以加唤醒处理逻辑
			
			BMS_GlobalParam.SysMode = BMS_MODE_STANDBY;
			BMS_INFO("Wake Up");
		}
		return;
	}

	
	if (BMS_MonitorData.BatteryCurrent < 0.02 && BMS_MonitorData.BatteryCurrent > -0.02)
	{
		BMS_GlobalParam.SysMode = BMS_MODE_STANDBY;	
		
		if (StandbyCount >= BMS_ENTRY_SLEEP_TIME * 60000)
		{
			// 没有电芯正在均衡的情况下才进入睡眠
			if (osSemaphoreAcquire(BalanceSem, 0) == RT_EOK)
			{
				// 可以加睡眠低功耗处理逻辑
				
				StandbyCount = 0;
				
				BMS_GlobalParam.SysMode = BMS_MODE_SLEEP;

				osSemaphoreRelease(BalanceSem);
				
				BMS_INFO("Entry Sleep Mode");
			}
		}
		else
		{
			StandbyCount += MONITOR_TASK_PERIOD;
		}


		// 调试用
		if (SysModeBackup != BMS_MODE_STANDBY)
		{
			SysModeBackup = BMS_MODE_STANDBY;
			BMS_INFO("Entry Standby Mode");
		}
	}
	else if (BMS_MonitorData.BatteryCurrent >= 0.02)
	{
		StandbyCount = 0;
		BMS_GlobalParam.SysMode = BMS_MODE_CHARGE;

		// 调试用
		if (SysModeBackup != BMS_MODE_CHARGE)
		{
			SysModeBackup = BMS_MODE_CHARGE;
			BMS_INFO("Entry Charge Mode");
		}
	}
	else if (BMS_MonitorData.BatteryCurrent <= -0.02)
	{
		StandbyCount = 0;
		BMS_GlobalParam.SysMode = BMS_MODE_DISCHARGE;

		// 调试用
		if (SysModeBackup != BMS_MODE_DISCHARGE)
		{
			SysModeBackup = BMS_MODE_DISCHARGE;
			BMS_INFO("Entry Discharge Mode");
		}
	}
}










void BMS_MonitorStateCellVoltage(BMS_StateTypedef NewState)
{
	if (NewState == BMS_STATE_ENABLE)
	{
		FlagCellVoltage = true;
	}
	else if (NewState == BMS_STATE_DISABLE)
	{
		FlagCellVoltage = false;
	}
}

void BMS_MonitorStateBatVoltage(BMS_StateTypedef NewState)
{
	if (NewState == BMS_STATE_ENABLE)
	{
		FlagBatVoltage = true;
	}
	else if (NewState == BMS_STATE_DISABLE)
	{
		FlagBatVoltage = false;
	}
}

void BMS_MonitorStateCellTemp(BMS_StateTypedef NewState)
{
	if (NewState == BMS_STATE_ENABLE)
	{
		FlagCellTemp = true;
	}
	else if (NewState == BMS_STATE_DISABLE)
	{
		FlagCellTemp = false;
	}	
}

void BMS_MonitorStateBatCurrent(BMS_StateTypedef NewState)
{
	if (NewState == BMS_STATE_ENABLE)
	{
		FlagBatCurrent = true;
	}
	else if (NewState == BMS_STATE_DISABLE)
	{
		FlagBatCurrent = false;
	}	
}





void BMS_MonitorHwCurrent(void)
{
	FlagSampleIntCur = true;
}

