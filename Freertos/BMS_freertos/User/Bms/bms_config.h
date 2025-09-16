#ifndef __BMS_CONFIG_H__
#define __BMS_CONFIG_H__



#include "bms_hal_config.h"




// 最多支持多少节电芯
// BQ76920:3~5
// BQ76930:6~10
// BQ76940:9~15
#define BMS_CELL_MAX	5


// 最多支持几路温度
// BQ76920:1
// BQ76930:2
// BQ76940:3
#define BMS_TEMP_MAX	1


// 温度测量范围,具体值需要根据热敏电阻和BQ芯片测量范围决定
#define BMS_TEMP_MEASURE_MAX	125
#define	BMS_TEMP_MEASURE_MIN	-55

// 当测量出来的温度值上面这个范围时,用这个无效值来代替
#define BMS_TEMP_INVALID_VALUE	255

// 默认电池额定容量值(A/H)
// 这个值没有实际用容量测仪校准过,是卖家口头说的
#define BMS_BATTERY_CAPACITY	2.2

// 在待机模式下静止多少时间进入睡眠模式(Min),睡眠低功耗处理暂未考虑
#define BMS_ENTRY_SLEEP_TIME	60



/***************************** 电池保护相关参数 ***********************************/
// 三元锂电池(Ternary lithium battery)默认参数
#define TLB_OV_PROTECT			4.20	// 单体过压保护电压
#define TLB_OV_RELIEVE			4.18	// 单体过压恢复电压
#define TLB_UV_PROTECT			3.10	// 单体欠压保护电压
#define TLB_UV_RELIEVE			3.15	// 单体欠压恢复电压
#define TLB_SHUTDOWN_VOLTAGE	3.08	// 自动关机电压
#define TLB_BALANCE_VOLTAGE		3.30	// 均衡起始电压



// 磷酸铁锂电池(lithium iron phosphate battery)默认参数
#define LIPB_OV_PROTECT			3.60	// 单体过压保护电压
#define LIPB_OV_RELIEVE			3.55	// 单体过压恢复电压
#define LIPB_UV_PROTECT			2.60	// 单体欠压保护电压
#define LIPB_UV_RELIEVE			2.65	// 单体欠压恢复电压
#define LIPB_SHUTDOWN_VOLTAGE	2.50 	// 自动关机电压
#define LIPB_BALANCE_VOLTAGE	3.00	// 均衡起始电压


// 钛酸锂电池(Lithium titanate battery)默认参数
#define LTB_OV_PROTECT			2.70	// 单体过压保护电压
#define LTB_OV_RELIEVE			2.65	// 单体过压恢复电压
#define LTB_UV_PROTECT			1.80	// 单体欠压保护电压
#define LTB_UV_RELIEVE			1.85	// 单体欠压恢复电压
#define LTB_SHUTDOWN_VOLTAGE	1.70	// 自动关机电压
#define LTB_BALANCE_VOLTAGE		2.30	// 均衡起始电压



// 初始默认值
#define INIT_OV_PROTECT			TLB_OV_PROTECT			// 单体过压保护电压(V)(注意BQ769X0 OV范围：3.15~4.70V)
#define INIT_OV_RELIEVE			TLB_OV_RELIEVE			// 单体过压恢复电压(V)
#define INIT_UV_PROTECT			TLB_UV_PROTECT			// 单体欠压保护电压(V)(注意BQ769X0 UV范围：1.58~3.10V)
#define INIT_UV_RELIEVE			TLB_UV_RELIEVE			// 单体欠压恢复电压(V)

#define INIT_SHUTDOWN_VOLTAGE	TLB_SHUTDOWN_VOLTAGE	// 自动关机电压(V),未考虑预留
#define INIT_BALANCE_VOLTAGE	TLB_BALANCE_VOLTAGE		// 均衡起始电压(V)

#define INIT_BALANCE_CURRENT_MAX	0.6		// 最大均衡电流(A),未考虑预留
#define	INIT_OCC_MAX				2.2		// 最大充电电流(A)
#define	INIT_OCD_MAX				2.2		// 最大放电电流(A)


#define INIT_OV_DELAY		BMS_OV_DELAY_2s		// 充电过压保护延时时间	OV:Over	Voltage
#define INIT_UV_DELAY 		BMS_UV_DELAY_4s		// 放电欠压保护延时时间 UV:Under Voltage

#define INIT_OCD_DELAY		BMS_OCD_DELAY_320ms // 放电过流延时时间(S) OCD:Over Current Discharge
#define INIT_OCD_RELIEVE	60					// 放电过流解除时间(S)

#define INIT_SCD_DELAY		BMS_SCD_DELAY_100us	// 放电短路延时时间(us) SCD:Short Circuit Discharge
#define INIT_SCD_RELIEVE	60					// 放电短路解除时间(S)

#define INIT_OCC_DELAY		1		// 充电过流延时时间(S) OCC:Over Current Charge
#define INIT_OCC_RELIEVE	60		// 充电过流解除时间(S)

#define INIT_OTC_PROTECT	70		// 充电过温保护(℃) OTC:Over Temperature Charge
#define INIT_OTC_RELIEVE	60		// 充电过温解除(℃)

#define INIT_OTD_PROTECT	70		// 放电过温保护(℃) OTD:Over Temperature Discharge
#define INIT_OTD_RELIEVE	60		// 放电过温解除(℃)

#define INIT_LTC_PROTECT	-20		// 充电低温保护(℃) LTC:Low Temperature Charge
#define INIT_LTC_RELIEVE	-10		// 充电低温解除(℃)

#define INIT_LTD_PROTECT	-20		// 放电低温保护(℃) LTD:Low Temperature Discharge
#define INIT_LTD_RELIEVE	-10		// 放电低温解除(℃)	









#define SOC_STOP_CHG_VALUE		1		// 停止充电SOC值
#define SOC_START_CHG_VALUE		0.99	// 启动充电SOC值
#define SOC_STOP_DSG_VALUE		0		// 停止放电SOC值
#define SOC_START_DSG_VALUE		0.01	// 启动放电SOC值

#define BALANCE_DIFFE_VOLTAGE	0.05	// 均衡差异电压(V)
#define BALANCE_CYCLE_TIME		30		// 均衡周期时间(s)
#define BALANCE_VOLT_RISE_DELAY 5000	// 均衡电压回升延时(ms)
/*************************************************************************************/


#endif


