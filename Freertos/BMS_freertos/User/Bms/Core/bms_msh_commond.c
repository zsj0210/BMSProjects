#include <rtthread.h>
#include <stdio.h>

#include "bms_hal_control.h"
#include "bms_hal_monitor.h"

#include "bms_monitor.h"
#include "bms_energy.h"
#include "bms_global.h"
#include "bms_debug.h"




/***************************************** 控制类 *********************************/

/* 睡眠唤醒由命令控制不合理，应该由系统控制
static void BMS_CmdWakeup(void)
{
	BMS_HalCtrlWakeup();
}
MSH_CMD_EXPORT(BMS_CmdWakeup, wakeup);


static void BMS_CmdSleep(void)
{
	BMS_HalCtrlSleep();
}
MSH_CMD_EXPORT(BMS_CmdSleep, sleep);
*/


static void BMS_CmdOpenDSG(void)
{
	BMS_GlobalParam.Discharge = BMS_STATE_ENABLE;
}
MSH_CMD_EXPORT(BMS_CmdOpenDSG, Open DSG);



static void BMS_CmdCloseDSG(void)
{
	BMS_GlobalParam.Discharge = BMS_STATE_DISABLE;
}
MSH_CMD_EXPORT(BMS_CmdCloseDSG, Close DSG);



static void BMS_CmdOpenCHG(void)
{
	BMS_GlobalParam.Charge = BMS_STATE_ENABLE;
}
MSH_CMD_EXPORT(BMS_CmdOpenCHG, Open CHG);



static void BMS_CmdCloseCHG(void)
{
	BMS_GlobalParam.Charge = BMS_STATE_DISABLE;
}
MSH_CMD_EXPORT(BMS_CmdCloseCHG, Close CHG);





static void BMS_CmdOpenBalance(void)
{
	BMS_GlobalParam.Balance = BMS_STATE_ENABLE;
}
MSH_CMD_EXPORT(BMS_CmdOpenBalance, Open Balance);



static void BMS_CmdCloseBalance(void)
{
	rt_sem_release(BalanceSem);
	BMS_GlobalParam.Balance = BMS_STATE_DISABLE;
}
MSH_CMD_EXPORT(BMS_CmdCloseBalance, Close Balance);





static void BMS_CmdLoadDetect(void)
{
	if (Bms_HalMonitorLoadDetect() == true)
	{
		BMS_INFO("Load Detected");
	}
	else
	{		
		BMS_INFO("No Load Was Detected");
	}
}
MSH_CMD_EXPORT(BMS_CmdLoadDetect, Load Detect);


/**********************************************************************************/










/***************************************** 传感数据 *********************************/

/**********************************************************************************/





