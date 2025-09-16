#include "bms_app.h"


#include "bms_hal_control.h"

#include "bms_monitor.h"
#include "bms_protect.h"
#include "bms_analysis.h"
#include "bms_energy.h"
#include "bms_info.h"
#include "bms_comm.h"
#include "bms_global.h"

#include "drv_soft_i2c.h"
#include "drv_softi2c_bq769x0.h"



void BMS_SysInitialize(void)
{
	#if 1
	BQ769X0_InitDataTypedef InitData;

	InitData.AlertOps.ocd 	 = BMS_ProtectHwOCD;
	InitData.AlertOps.scd 	 = BMS_ProtectHwSCD;
	InitData.AlertOps.ov	 = BMS_ProtectHwOV;
	InitData.AlertOps.uv 	 = BMS_ProtectHwUV;	

	// 使用硬件中断通知,如果烧写程序后必须重新上下电一次BQ芯片或者复位
	InitData.AlertOps.cc 	 = BMS_MonitorHwCurrent;
	//InitData.AlertOps.cc 	 = NULL;

	// 这两个中断会造成系统故障
	// 第一个报警时设备故障,表示BQ芯片有问题了
	// 第二个报警可能存在被外界电磁信号干扰造成误判,之前出现过,换了个跟官方一样阻值的电阻就没出现过了
	InitData.AlertOps.device = BMS_ProtectHwDevice;
	InitData.AlertOps.ovrd 	 = BMS_ProtectHwOvrd;

	InitData.ConfigData.SCDDelay	 = (BQ769X0_SCDDelayTypedef)INIT_SCD_DELAY;
	InitData.ConfigData.OCDDelay	 = (BQ769X0_OCDDelayTypedef)INIT_OCD_DELAY;
	InitData.ConfigData.UVDelay	 	 = (BQ769X0_OVDelayTypedef)INIT_UV_DELAY;
	InitData.ConfigData.OVDelay	 	 = (BQ769X0_UVDelayTypedef)INIT_OV_DELAY;
	InitData.ConfigData.UVPThreshold = INIT_UV_PROTECT * 1000;
	InitData.ConfigData.OVPThreshold = INIT_OV_PROTECT * 1000;


	// 硬件初始化
	I2C_BusInitialize();
	BQ769X0_Initialize(&InitData);
#endif

	// 软件初始化
	BMS_MonitorInit();	// 电池监控初始化
	BMS_ProtectInit();	// 电池保护初始化
	BMS_AnalysisInit();	// 电池分析初始化
	BMS_EnergyInit();	// 能量管理初始化
	BMS_InfoInit();		// 信息管理初始化
//	BMS_CommInit();		// 通信管理初始化
}


