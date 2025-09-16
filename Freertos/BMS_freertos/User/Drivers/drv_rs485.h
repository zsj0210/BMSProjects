#ifndef __DRV_RS485_H__
#define __DRV_RS485_H__


typedef enum
{
	RS485_ENABLE_PWR = 0u,
	RS485_DISABLE_PWR,
}RS485_PwrStateTypedef;



typedef enum
{
	RS485_RX = 0u,
	RS485_TX,
}RS485_TransmitStateTypedef;


void RS485_Initialize(void);
void RS485_PwrState(RS485_PwrStateTypedef NewState);
void RS485_TransmitState(RS485_TransmitStateTypedef NewState);


void RS485_TransmitTest(void);

#endif



