#include "drv_can.h"

#include <rtthread.h>
#include <string.h>

#include "main.h"
#include "can.h"



void CAN_Initialize(void)
{
	CAN_FilterTypeDef filter = {0};

	CAN_PwrState(CAN_ENABLE_PWR);
	
    filter.FilterActivation = ENABLE;				// 是否使能筛选器
    filter.FilterMode = CAN_FILTERMODE_IDMASK;		// 筛选器模式
    filter.FilterScale = CAN_FILTERSCALE_32BIT;		// 筛选器的位数
    filter.FilterBank = 0;							// 筛选器编号
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;	// 设置经过筛选后数据存储到哪个接收 FIFO
    filter.FilterIdLow = 0;			// CAN_FxR1 寄存器的低 16 位
    filter.FilterIdHigh = 0;		// CAN_FxR1 寄存器的高 16 位
    filter.FilterMaskIdLow = 0;		// CAN_FxR2 寄存器的低 16 位
	filter.FilterMaskIdHigh = 0;	// CAN_FxR2 寄存器的高 16 位

	// 配置滤波器
	if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK)
	{
		rt_kprintf("CAN Config Filter Fail!\r\n");
	}

	// 打开CAN RX0中断
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);  

	// 开启CAN
	if (HAL_CAN_Start(&hcan) != HAL_OK)
	{
		rt_kprintf("CAN Start Fail!\r\n");
	}
}


void CAN_SendData(uint32_t IdType,uint32_t StdId,uint32_t ExtId,uint8_t *pData,uint32_t length)
{
    uint32_t TxMailbox;
	uint8_t buffer[8] = {0};
	
    CAN_TxHeaderTypeDef TxHeader;
    
    TxHeader.StdId = StdId;			// 标准ID
    TxHeader.ExtId = ExtId;			// 扩展ID
    TxHeader.IDE = IdType;			// ID类型
    TxHeader.RTR = CAN_RTR_DATA;	// 数据帧/远程帧
    TxHeader.DLC = length;			// 数据长度
	TxHeader.TransmitGlobalTime = DISABLE;
	
	memcpy(buffer, pData, length);
 
	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, buffer, &TxMailbox) != HAL_OK)
	{
		rt_kprintf("CAN Send Data Fail!\r\n");
	}
}


void CAN_PwrState(CAN_PwrStateTypedef NewState)
{
	HAL_GPIO_WritePin(CAN_PWREN_GPIO_Port, CAN_PWREN_Pin, (GPIO_PinState)NewState);
}







void CAN_SendTest(void)
{
	static uint8_t send_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	static uint8_t id_type_std = 0;


	send_data[0]++;
	send_data[1]++;
	send_data[2]++;
	send_data[3]++;
	send_data[4]++;
	send_data[5]++;
	send_data[6]++;
	send_data[7]++;
	
	if(id_type_std == 1)
	{
		CAN_SendData(CAN_ID_STD, 0x123, 0x321, send_data, 8);
		id_type_std = 0;
	}
	else
	{
		CAN_SendData(CAN_ID_EXT, 0x123, 0x321, send_data, 8);
		id_type_std = 1;
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	uint8_t buffer[8], i;
	CAN_RxHeaderTypeDef hCAN1_RxHeader;

    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hCAN1_RxHeader, buffer) == HAL_OK)
    {
        rt_kprintf("\nGet Rx Message Success!!\r\nData:");
        
        for(i=0; i<8; i++)
        {
            rt_kprintf("%d", buffer[i]);
       	}
    }
}

