#ifndef __DRV_CAN_H__
#define __DRV_CAN_H__

#include <stdint.h>

typedef enum
{
	CAN_ENABLE_PWR = 0u,
	CAN_DISABLE_PWR,
}CAN_PwrStateTypedef;



void CAN_Initialize(void);
void CAN_SendData(uint32_t IdType,uint32_t StdId,uint32_t ExtId,uint8_t *pData,uint32_t length);
void CAN_PwrState(CAN_PwrStateTypedef NewState);

void CAN_SendTest(void);


#endif

