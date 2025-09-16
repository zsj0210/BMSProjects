#include "bms_hal_control.h"


#include "drv_softi2c_bq769x0.h"




// BMS唤醒
void BMS_HalCtrlWakeup(void)
{
	BQ769X0_Wakeup();
}

// 控制BMS进入睡眠模式
void BMS_HalCtrlSleep(void)
{
	BQ769X0_EntryShip();
}

// 控制放电状态
void BMS_HalCtrlDischarge(BMS_StateTypedef NewState)
{
	BQ769X0_ControlDSGOrCHG(DSG_CONTROL, (BQ769X0_StateTypedef)NewState);
}

// 控制充电状态
void BMS_HalCtrlCharge(BMS_StateTypedef NewState)
{
	BQ769X0_ControlDSGOrCHG(CHG_CONTROL, (BQ769X0_StateTypedef)NewState);
}

// 控制单节或多节电芯均衡,最多可以支持32节
void BMS_HalCtrlCellsBalance(BMS_CellIndexTypedef CellIndex, BMS_StateTypedef NewState)
{
	BQ769X0_CellBalanceControl((BQ769X0_CellIndexTypedef)CellIndex, (BQ769X0_StateTypedef)NewState);
}


