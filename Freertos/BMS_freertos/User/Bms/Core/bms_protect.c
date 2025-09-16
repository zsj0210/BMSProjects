#define BMS_DBG_TAG "Protect"

#include <stdio.h>
#include <stdbool.h>
#include <rtthread.h>

#include "bms_protect.h"

#include "bms_hal_monitor.h"
#include "bms_hal_control.h"
#include "bms_monitor.h"
#include "bms_global.h"
#include "bms_debug.h"




// thread config
#define PROTECT_TASK_STACK_SIZE	512
#define PROTECT_TASK_PRIORITY	10
#define PROTECT_TASK_TIMESLICE	25

#define PROTECT_TASK_PERIOD		200



BMS_ProtectAlertTypedef BMS_ProtectAlert = FlAG_ALERT_NO;
BMS_ProtectParamTypedef BMS_ProtectParam = 
{
	.ShoutdownVoltage = INIT_SHUTDOWN_VOLTAGE,

	.OVProtect	= INIT_OV_PROTECT,
	.OVRelieve	= INIT_OV_RELIEVE,
	.UVProtect	= INIT_UV_PROTECT,
	.UVRelieve	= INIT_UV_RELIEVE,

	.OCCProtect = INIT_OCC_MAX,
	.OCDProtect = INIT_OCD_MAX,

	.OVDelay	= INIT_OV_DELAY,
	.UVDelay	= INIT_UV_DELAY,
	.OCDDelay	= INIT_OCD_DELAY,
	.SCDDelay	= INIT_SCD_DELAY,

	.OCDRelieve = INIT_OCD_RELIEVE,
	.SCDRelieve = INIT_SCD_RELIEVE,
	.OCCDelay	= INIT_OCC_DELAY,
	.OCCRelieve = INIT_OCC_RELIEVE,

	.OTCProtect = INIT_OTC_PROTECT,
	.OTCRelieve = INIT_OTC_RELIEVE,
	.OTDProtect = INIT_OTD_PROTECT,
	.OTDRelieve = INIT_OTD_RELIEVE,

	.LTCProtect = INIT_LTC_PROTECT,
	.LTCRelieve = INIT_LTC_RELIEVE,
	.LTDProtect = INIT_LTD_PROTECT,
	.LTDRelieve = INIT_LTD_RELIEVE,
};

static osTimerId_t pTimerProtect;
static BMS_ProtectStateTypedef ProtectState = PROTECT_STATE_MONITOR;

const osThreadAttr_t protectTask_attributes = {
  .name = "protectTask",
  .stack_size = PROTECT_TASK_STACK_SIZE,
  .priority = (osPriority_t) osPriorityNormal4,
};

const osTimerAttr_t protectTimer_attributes = {
  .name = "protectTimer",
};


static void BMS_ProtectTimerEntry(void *paramter);
static void BMS_ProtectTaskEntry(void *paramter);

static void BMS_ProtectSwMonitor(void);
static void BMS_ProtectRelieveWait(void);
static void BMS_ProtectRelieve(void);

void BMS_ProtectHwMonitor(void);


// ��������ĳ�ʼ��
void BMS_ProtectInit(void)
{
	osThreadId_t thread;

	
 	thread = osThreadNew(BMS_ProtectTaskEntry, NULL, &protectTask_attributes);

	if (thread == NULL)
	{
		BMS_ERROR("Create Task Fail");
	}

	
	pTimerProtect = osTimerNew(BMS_ProtectTimerEntry, osTimerOnce, NULL, &protectTimer_attributes);

	if (pTimerProtect == NULL)
	{
		BMS_ERROR("Create Timer Fail");
	}
}


// �����������
static void BMS_ProtectTaskEntry(void *paramter)
{
	(void)paramter;
	
	while(1)
	{
		switch(ProtectState)
		{
			case PROTECT_STATE_MONITOR:
			{
				BMS_ProtectSwMonitor();
				BMS_ProtectHwMonitor();
			}break;
			
			case PROTECT_STATE_RELIEVE_WAIT:
			{
				BMS_ProtectRelieveWait();
			}break;

			case PROTECT_STATE_RELIEVE:
			{
				BMS_ProtectRelieve();
			}break;
		}
		osDelay(PROTECT_TASK_PERIOD);
	}
}

// ���ڱ�������Ķ�ʱ���ص����
static void BMS_ProtectTimerEntry(void *paramter)
{
	(void)paramter;
	ProtectState = PROTECT_STATE_RELIEVE;

	BMS_INFO("Protect Timer Tigger");
}

// �����û���������Ķ�ʱ��
static void BMS_ProtectStartTimer(uint32_t sec)
{
	uint32_t tick;

	tick = sec * 1000;
	osTimerStart(pTimerProtect, tick);

	BMS_INFO("Protect Timer Start");
}










// �����:���������¡�����
// �������ֱ��������һ�־ͻ�رճ��,ֱ���ﵽ�ָ�����
static void BMS_ChargeMonitor(void)
{
	static uint32_t ProtectCount = 0;

	if (BMS_MonitorData.BatteryCurrent > BMS_ProtectParam.OCCProtect)
	{
		// ����
		ProtectCount += PROTECT_TASK_PERIOD;
		if (ProtectCount / 60 >= BMS_ProtectParam.OCCDelay)
		{			
			BMS_HalCtrlCharge(BMS_STATE_DISABLE);
			BMS_ProtectStartTimer(BMS_ProtectParam.OCCRelieve);
			BMS_ProtectAlert = FlAG_ALERT_OCC;
			ProtectState = PROTECT_STATE_RELIEVE_WAIT;

			BMS_WARNING("OCC Tigger");
		}
	}
	else if (BMS_MonitorData.CellTempEffectiveNumber == 0)
	{
		// ��Ч���¶Ȳ����бȽ�
		return;
	}
	else if (BMS_MonitorData.CellTemp[BMS_MonitorData.CellTempEffectiveNumber-1] > BMS_ProtectParam.OTCProtect)
	{
		// ����
		BMS_HalCtrlCharge(BMS_STATE_DISABLE);
		BMS_ProtectAlert = FlAG_ALERT_OTC;	
		ProtectState = PROTECT_STATE_RELIEVE_WAIT;
		
		BMS_WARNING("OTC Tigger");
	}
	else if (BMS_MonitorData.CellTemp[0] < BMS_ProtectParam.LTCProtect)
	{
		// ����
		BMS_HalCtrlCharge(BMS_STATE_DISABLE);
		BMS_ProtectAlert = FlAG_ALERT_LTC;	
		ProtectState = PROTECT_STATE_RELIEVE_WAIT;		

		BMS_WARNING("LTC Tigger");
	}
	else
	{
		// ��λ����
		ProtectCount = 0;
	}
}

// �ŵ���
static void BMS_DischargeMonitor(void)
{
	if (BMS_MonitorData.CellTempEffectiveNumber == 0)
	{
		// ����Ч���¶Ȳ����бȽ�
		return;
	}
	else if (BMS_MonitorData.CellTemp[BMS_MonitorData.CellTempEffectiveNumber-1] > BMS_ProtectParam.OTDProtect)
	{
		// ����
		BMS_HalCtrlDischarge(BMS_STATE_DISABLE);
		BMS_ProtectAlert = FlAG_ALERT_OTD;
		ProtectState = PROTECT_STATE_RELIEVE_WAIT;

		BMS_WARNING("OTD Tigger");
	}
	else if (BMS_MonitorData.CellTemp[0] < BMS_ProtectParam.LTDProtect)
	{
		// ����
		BMS_HalCtrlDischarge(BMS_STATE_DISABLE);		
		BMS_ProtectAlert = FlAG_ALERT_LTD;	
		ProtectState = PROTECT_STATE_RELIEVE_WAIT;
		

		BMS_WARNING("LTD Tigger");
	}
}


// ����������
static void BMS_ProtectSwMonitor(void)
{
	switch(BMS_GlobalParam.SysMode)
	{
		case BMS_MODE_CHARGE:
		{
			BMS_ChargeMonitor();
		}break;

		case BMS_MODE_DISCHARGE:
		{
			BMS_DischargeMonitor();
		}break;

		case BMS_MODE_STANDBY:
		{
			//BMS_StandbyMonitor();
		}break;

		case BMS_MODE_SLEEP:
		{
			// ˯����ʱûʲô�ɼ�ص�
		}break;
		
		default:;break;
	}
}

// ��������ȴ����ػָ�����
static void BMS_ProtectRelieveWait(void)
{
	switch (BMS_ProtectAlert)
	{
		case FlAG_ALERT_OV:
		{
			if (BMS_MonitorData.CellData[BMS_CELL_MAX-1].CellVoltage < BMS_ProtectParam.OVRelieve)
			{
				ProtectState = PROTECT_STATE_RELIEVE;

				BMS_INFO("OV Relieve");
			}
		}break;

		case FlAG_ALERT_UV:
		{
			if (BMS_MonitorData.CellData[0].CellVoltage > BMS_ProtectParam.UVRelieve)
			{
				ProtectState = PROTECT_STATE_RELIEVE;

				BMS_INFO("UV Relieve");
			}
		}break;

		case FlAG_ALERT_OTC:
		{
			if (BMS_MonitorData.CellTemp[BMS_TEMP_MAX-1] < BMS_ProtectParam.OTCRelieve)
			{
				ProtectState = PROTECT_STATE_RELIEVE;

				BMS_INFO("OTC Relieve");
			}
		}break;

		case FlAG_ALERT_OTD:
		{
			if (BMS_MonitorData.CellTemp[BMS_TEMP_MAX-1] < BMS_ProtectParam.OTDRelieve)
			{
				ProtectState = PROTECT_STATE_RELIEVE;

				BMS_INFO("OTD Relieve");
			}
		}break;

		case FlAG_ALERT_LTC:
		{
			if (BMS_MonitorData.CellTemp[0] > BMS_ProtectParam.LTCRelieve)
			{
				ProtectState = PROTECT_STATE_RELIEVE;

				BMS_INFO("LTC Relieve");
			}
		}break;

		case FlAG_ALERT_LTD:
		{
			if (BMS_MonitorData.CellTemp[0] > BMS_ProtectParam.LTDRelieve)
			{
				ProtectState = PROTECT_STATE_RELIEVE;

				BMS_INFO("LTD Relieve");
			}
		}break;

		default:
		{

		}
		break;
	}
}


// �������
static void BMS_ProtectRelieve(void)
{	
	// ��ѹ��Ƿѹ���ָ�
	switch(BMS_ProtectAlert)
	{
		case FlAG_ALERT_OCC:
		case FlAG_ALERT_OTC:
		case FlAG_ALERT_LTC:
			BMS_HalCtrlCharge(BMS_GlobalParam.Charge);
		break;

		case FlAG_ALERT_OCD:
		case FlAG_ALERT_OTD:
		case FlAG_ALERT_LTD:
		case FlAG_ALERT_SCD:
			BMS_HalCtrlDischarge(BMS_GlobalParam.Discharge);
		break;

		default:;break;
	}
	
	BMS_ProtectAlert = FlAG_ALERT_NO;	
	ProtectState = PROTECT_STATE_MONITOR;

	BMS_INFO("Protect Relieve");
}












// Ӳ���������
void BMS_ProtectHwMonitor(void)
{
	switch(BMS_ProtectAlert)
	{
		case FlAG_ALERT_OCD:
		{			
			BMS_ProtectStartTimer(BMS_ProtectParam.OCDRelieve);
			ProtectState = PROTECT_STATE_RELIEVE_WAIT;

			BMS_WARNING("OCD Tigger");
		}break;

		case FlAG_ALERT_SCD:
		{			
			BMS_ProtectStartTimer(BMS_ProtectParam.SCDRelieve);
			ProtectState = PROTECT_STATE_RELIEVE_WAIT;

			BMS_WARNING("SCD Tigger");
		}break;

		case FlAG_ALERT_OV:
		{
			ProtectState = PROTECT_STATE_RELIEVE_WAIT;
			
			BMS_WARNING("OV Tigger");
		}break;

		case FlAG_ALERT_UV:
		{
			ProtectState = PROTECT_STATE_RELIEVE_WAIT;

			BMS_WARNING("UV Tigger");
		}break;

		default:
		{

		}break;
	}
}


// �ŵ����(OCD)Ӳ������
void BMS_ProtectHwOCD(void)
{
	if (BMS_ProtectAlert == FlAG_ALERT_NO)
	{
		BMS_ProtectAlert = FlAG_ALERT_OCD;
	}
}

// �ŵ��·(SCD)Ӳ������
void BMS_ProtectHwSCD(void)
{
	if (BMS_ProtectAlert == FlAG_ALERT_NO)
	{
		BMS_ProtectAlert = FlAG_ALERT_SCD;
	}
}

// ����ѹ(OV)Ӳ������
void BMS_ProtectHwOV(void)
{
	if (BMS_ProtectAlert == FlAG_ALERT_NO)
	{
		BMS_ProtectAlert = FlAG_ALERT_OV;
	}
}

// �ŵ�Ƿѹ(UV)Ӳ������
void BMS_ProtectHwUV(void)
{
	if (BMS_ProtectAlert == FlAG_ALERT_NO)
	{
		BMS_ProtectAlert = FlAG_ALERT_UV;
	}
}


void BMS_ProtectHwDevice(void)
{
	BMS_INFO("BMS_ProtectHwDevice");
}


void BMS_ProtectHwOvrd(void)
{
	BMS_INFO("BMS_ProtectHwOvrd");
}



