#include "drv_softi2c_bq769x0.h"

#include "main.h"

#include "drv_soft_i2c.h"


// 报警回调接口
static BQ769X0_AlertOpsTypedf AlertOps;


/* ADC增益 */
static float Gain = 0;		
static int16_t iGain = 0;
static int8_t Adcoffset;

static uint8_t TemNA = 6;
static uint8_t TempSampleMode = 0;  // 温度采样模式 0:热敏电阻  1:IC温度
//--------20,-16,-12,-8,-4,0,4,8, 100------------//
// 10K NTC 5% 热敏电阻温度转换表
static const uint16_t TemD[31] =
{
	2890,2817,2735,2642,2541,2431,2318,2202,2127,2007,  
	1846,1687,1546,1422,1309,1202,1099,1001,907,817,
	727,677,613,550,496,452,414,379,345,311,277
};



// 寄存器组
static RegisterGroup Registers = {0};


// 18650 1C放电倍率是指电池以1小时时间放完额定容量
// 假如电池额定容量为2200mA/H ,那么1C放电电流是2.2A
// 18650短路电流阈值一般为电池的5C放电速率
// 短路电流一般设置为10A（搜索别人实际测过的经验值）


// 放电短路保护阈值，选择44还是89是根据RSNS位配置的，为1则加倍阈值否则不加倍
// 假如SCD预设为10A 根据我的电路分流电阻规格是5mΩ
// 计算  	SCDThresh = 10A * 5mΩ = 50mV
// 向下取值应该取 SCD_THRESH_89mV_44mV  并且RSNS设置为0
// 取44mV 实际计算 44 / 5 = 8.8A 这才是真正的阈值保护电流
// 设置为56mV 放电短路阈值电流是11.2A
static const uint8_t SCDThresh = SCD_THRESH_89mV_44mV;




// 放电过流保护阈值，一般设置为2A
// 选择11还是22是根据RSNS位配置的，为1则加倍阈值否则不加倍
// 假如OCD预设为2.2A 根据我的电路分流电阻规格是5mΩ
// 计算  	OCDThresh = 2.2A * 5mΩ = 11mV
// 取值应该取 OCD_THRESH_22mV_11mV  并且RSNS设置为0
// 设置为8mV 放电过流阈值电流是1.6A
// 设置为14mV 放电过流阈值电流是2.8A
static const uint8_t OCDThresh = OCD_THRESH_22mV_11mV;

// OCD_THRESH_17mV_8mV  OCD_THRESH_22mV_11mV






/*
// 放电短路保护延时时间
static const uint8_t SCDDelay = SCD_DELAY_100us;

// 放电过流保护延时时间
static const uint8_t OCDDelay = OCD_DELAY_320ms;

// 过压保护延迟
static const uint8_t OVDelay = OV_DELAY_2s;

// 欠压保护延迟
static const uint8_t UVDelay = UV_DELAY_4s;




// 三元锂和磷酸铁锂区别看标称电压,三元锂:3.6V或3.7V						磷酸铁锂:3.2V
// 三元锂过充终止:4.2V  				磷酸铁锂过充终止:3.6V
// 三元锂过放截止:3.2V  				磷酸铁锂过放截止:2.5V
// 以上值只是行业参考，锂电池过充、过放值还得根据电池卖家给的值

// BQ芯片可配置过压保护阈值,范围3.15~4.7V
static const uint16_t  OVPThreshold = 4200;		

// BQ芯片可配置欠压保护阈值,范围1.58~3.1V
static const uint16_t	UVPThreshold = 2900;
*/




// 分流电阻阻值,单位为欧
static float RsnsValue= 0.005;		




// 传感数据
BQ769X0_SampleDataTypedef BQ769X0_SampleData = {0};


static void BQ769X0_AlertHandler(void);



/*********************************** GPIO *********************************************/
/*
static void BQ769X0_GpioInit(void)
{

}
*/

static void BQ769X0_TS1_SetOutMode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = BQ769X0_TS1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(BQ769X0_TS1_GPIO_Port, &GPIO_InitStruct);
}

static void BQ769X0_TS1_SetInMode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
	
    GPIO_InitStruct.Pin = BQ769X0_TS1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(BQ769X0_TS1_GPIO_Port, &GPIO_InitStruct);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BQ769X0_ALERT_Pin)
    {
		BQ769X0_AlertHandler();
    }
}
/******************************************************************************************/


/************************************** utils **********************************************/

// 将采集到的热敏电阻ADC值，转换成实际的温度值
static int16_t TempChange(uint16_t uiADCV)
{	
	uint8_t ucA = 32, TemNK = 0;
	int16_t uiD = 0;
	
	TemNK = TemNA;
	if (uiADCV < TemD[TemNK])
	{	
		while (--ucA)
		{	
			if (TemNK < 30)
			{	
				TemNK++;
				if (uiADCV >= TemD[TemNK])
				{	
					uiD = -200 + TemNK * (int)40;
					uiD = uiD - ((int)40) * (uiADCV-TemD[TemNK]) / (TemD[TemNK-1]-TemD[TemNK]);
					break;
				}
			}
			else
			{
				uiD = 1000;		//100
				break;
			}
		}
	}
	else if (uiADCV >= TemD[TemNK-1])			
	{	
		while (--ucA)
		{	
			if (TemNK > 1)
			{	
				TemNK--;
				if(uiADCV < TemD[TemNK-1])
				{	
					uiD = -200 + TemNK * (int)40;
					uiD = uiD - ((int)40) * (uiADCV-TemD[TemNK]) / (TemD[TemNK-1]-TemD[TemNK]);
					break;
				}
			}
			else 
			{	
				uiD = -200;		//-30
				break;
			}
		}
	}
	else
	{	
		uiD = -200 + TemNK * (int)40;
		uiD = uiD - ((int)40) * (uiADCV - TemD[TemNK]) / (TemD[TemNK-1] - TemD[TemNK]);
	}
	TemNA = TemNK;
	
	if(uiD&0x8000)
	{
		uiD &= ~0x8000;
		uiD = 0x8000 - uiD;
		uiD = uiD * 90 / 100;
		uiD &= 0x7fff;
		//uiD = 0x8000-uiD;
		uiD |= 0x8000;
	}
	else 
	{
		uiD = uiD * 90 / 100;
	}

	return uiD;
}

// CRC8校验
static uint8_t CRC8(uint8_t *ptr, uint8_t len, uint8_t key)
{
	uint8_t i, crc=0;
	
	while (len-- != 0)
	{
		for (i = 0x80; i != 0; i /= 2)
		{
			if ((crc & 0x80) != 0)
			{
				crc *= 2;
				crc ^= key;
			}
			else
			{
				crc *= 2;
			}

			if ((*ptr & i) != 0)
			{
				crc ^= key;
			}
		}
		ptr++;
	}
	return(crc);
}
/******************************************************************************************/


/********************************** write and read ****************************************/

static bool BQ769X0_WriteRegisterByte(uint8_t Register, uint8_t data)
{
    uint8_t dataBuffer[2] = {Register, data};

	struct I2C_MessageTypeDef msg = {0};

	msg.addr = BQ769X0_I2C_ADDR;
	msg.flags = I2C_WR;	
	msg.buf = dataBuffer;
	msg.tLen = 2;
	
    if (I2C_TransferMessages(&i2c1, &msg, 1) != 1)
    {
		BQ769X0_ERROR("Write Register Byte Fail");

		return false;
    }

    return true;
}

static bool BQ769X0_WriteRegisterByteWithCRC(uint8_t Register, uint8_t data)
{
    uint8_t dataBuffer[4];
	struct I2C_MessageTypeDef msg = {0};

	dataBuffer[0] = BQ769X0_I2C_ADDR << 1;
	dataBuffer[1] = Register;
	dataBuffer[2] = data;	
	dataBuffer[3] = CRC8(dataBuffer, 3, CRC_KEY);

	msg.addr = BQ769X0_I2C_ADDR;
	msg.flags = I2C_WR;	
	msg.buf = dataBuffer + 1;
	msg.tLen = 3;

    if (I2C_TransferMessages(&i2c1, &msg, 1) != 1)
    {
		BQ769X0_ERROR("Write Register Byte With CRC Fail");

		return false;
    }

    return true;
}


static bool BQ769X0_WriteRegisterWordWithCRC(uint8_t Register, uint16_t data)
{
    uint8_t dataBuffer[6];
	struct I2C_MessageTypeDef msg = {0};

	dataBuffer[0] = BQ769X0_I2C_ADDR << 1;
	dataBuffer[1] = Register;
	dataBuffer[2] = LOW_BYTE(data);	
	dataBuffer[3] = CRC8(dataBuffer, 3, CRC_KEY);
	dataBuffer[4] = HIGH_BYTE(data);	
	dataBuffer[5] = CRC8(dataBuffer + 4, 1, CRC_KEY);		

	msg.addr = BQ769X0_I2C_ADDR;
	msg.flags = I2C_WR;	
	msg.buf = dataBuffer + 1;
	msg.tLen = 5;

    if (I2C_TransferMessages(&i2c1, &msg, 1) != 1)
    {
		BQ769X0_ERROR("Write Register Word With CRC Fail");
		return false;
    }

    return true;
}


static bool BQ769X0_WriteBlockWithCRC(uint8_t startAddress, uint8_t *buffer, uint8_t length)
{
	uint8_t index;
	uint8_t *bufferCRC, *pointer;
	struct I2C_MessageTypeDef msg = {0};

	bufferCRC = (uint8_t *)BQ769X0_MALLOC(2 * length + 2);
	if (NULL == bufferCRC)
	{
		BQ769X0_WARNING("Malloc Fail");

		return false;
    }


	pointer = bufferCRC;
	*pointer++ = BQ769X0_I2C_ADDR << 1;
	*pointer++ = startAddress;
	*pointer++ = *buffer;
	*pointer = CRC8(bufferCRC, 3, CRC_KEY);

	for(index = 1; index < length; index++)
	{
        pointer++;
        buffer++;
        *pointer = *buffer;
		*(pointer + 1) = CRC8(pointer, 1, CRC_KEY);
		pointer++;
	}

	msg.addr = BQ769X0_I2C_ADDR;
	msg.flags = I2C_WR;	
	msg.buf = bufferCRC + 1;
	msg.tLen = 2 * length + 1;

    if (I2C_TransferMessages(&i2c1, &msg, 1) != 1)
    {
		BQ769X0_ERROR("Write Register Block With CRC Fail");
		return false;
    }
	BQ769X0_FREE(bufferCRC);

    return true;	
}



static bool BQ769X0_ReadRegisterByte(uint8_t Register, uint8_t *data)
{  	
	struct I2C_MessageTypeDef msg[2] = {0};

	msg[0].addr = BQ769X0_I2C_ADDR;
	msg[0].flags = I2C_WR;	
	msg[0].buf = &Register;
	msg[0].tLen = 1;

	msg[1].addr = BQ769X0_I2C_ADDR;
	msg[1].flags = I2C_RD;	
	msg[1].buf = data;
	msg[1].tLen = 1;

    if (I2C_TransferMessages(&i2c1, msg, 2) != 2)
    {
		BQ769X0_ERROR("Read Register Byte Fail");
		return false;
    }

    return true;
}

static bool BQ769X0_ReadRegisterByteWithCRC(uint8_t Register, uint8_t *data)
{  	
	uint8_t readBuffer[2], crcInput[2], crcValue;
	struct I2C_MessageTypeDef msg[2] = {0};

	msg[0].addr = BQ769X0_I2C_ADDR;
	msg[0].flags = I2C_WR;	
	msg[0].buf = &Register;
	msg[0].tLen = 1;

	msg[1].addr = BQ769X0_I2C_ADDR;
	msg[1].flags = I2C_RD;	
	msg[1].buf = readBuffer;
	msg[1].tLen = 2;

    if (I2C_TransferMessages(&i2c1, msg, 2) != 2)
    {
		BQ769X0_ERROR("Read Register Byte With CRC Fail");

		return false;
    }


	crcInput[0] = (BQ769X0_I2C_ADDR << 1) + 1;
	crcInput[1] = readBuffer[0];

	crcValue = CRC8(crcInput, 2, CRC_KEY);
	if (crcValue != readBuffer[1])
	{
		BQ769X0_ERROR("Read Register Byte CRC Check Fail");
		return false;
    }

	*data = readBuffer[0];

	return true;
}



static bool BQ769X0_ReadRegisterWordWithCRC(uint8_t Register, uint16_t *data)
{  	
	uint8_t readBuffer[4], crcInput[2], crcValue;
	struct I2C_MessageTypeDef msg[2] = {0};

	msg[0].addr = BQ769X0_I2C_ADDR;
	msg[0].flags = I2C_WR;	
	msg[0].buf = &Register;
	msg[0].tLen = 1;

	msg[1].addr = BQ769X0_I2C_ADDR;
	msg[1].flags = I2C_RD;	
	msg[1].buf = readBuffer;
	msg[1].tLen = 4;

    if (I2C_TransferMessages(&i2c1, msg, 2) != 2)
    {
		BQ769X0_ERROR("Read Register Word With CRC Fail");

		return false;
    }

	crcInput[0] = (BQ769X0_I2C_ADDR << 1) + 1;
	crcInput[1] = readBuffer[0];

	crcValue = CRC8(crcInput, 2, CRC_KEY);
	if (crcValue != readBuffer[1])
	{
		BQ769X0_ERROR("Read Register Word CRC 1 Check Fail");

		return false;
	}

	crcValue = CRC8(readBuffer + 2, 1, CRC_KEY);
	if (crcValue != readBuffer[3])
	{
		BQ769X0_ERROR("Read Register Word CRC 2 Check Fail");

		return false;
	}
	*data = (readBuffer[2] << 8) | readBuffer[0];

	return true;
}

static bool BQ769X0_ReadBlockWithCRC(uint8_t Register, uint8_t *buffer, uint8_t length)
{  	
	uint8_t index, crcValue, crcInput[2];
	uint8_t *readData, *startData;
	struct I2C_MessageTypeDef msg[2] = {0};


    startData = (uint8_t *)BQ769X0_MALLOC(2 * length);
    if (NULL == startData)
    {
    	BQ769X0_WARNING("Malloc Fail"); 
    	
    	return false;
    }
    readData = startData;

	msg[0].addr = BQ769X0_I2C_ADDR;
	msg[0].flags = I2C_WR;	
	msg[0].buf = &Register;
	msg[0].tLen = 1;

	msg[1].addr = BQ769X0_I2C_ADDR;
	msg[1].flags = I2C_RD;	
	msg[1].buf = readData;
	msg[1].tLen = length * 2;

    if (I2C_TransferMessages(&i2c1, msg, 2) != 2)
    {
    	BQ769X0_FREE(startData);
		BQ769X0_ERROR("Read Register Block With CRC Cail");
		
		return false;
    }

	crcInput[0] = (BQ769X0_I2C_ADDR << 1) + 1;
	crcInput[1] = readData[0];

	crcValue = CRC8(crcInput, 2, CRC_KEY);
	readData++;
	if (crcValue != *readData)
	{
		BQ769X0_FREE(startData);
		BQ769X0_ERROR("Read Register Block CRC 1 Check Fail");
		
		return false;	
	}
	else
	{
		*buffer = *(readData - 1);
	}

	for(index = 1; index < length; index++)
	{
		readData++;
		crcValue = CRC8(readData, 1, CRC_KEY);
		readData++;
		buffer++;

		if (crcValue != *readData)
		{
			BQ769X0_FREE(startData);
			BQ769X0_ERROR("Read Register Block CRC Check Fail");
			
			return false;		
		}
		else
		{
			*buffer = *(readData - 1);
		}
	}

	BQ769X0_FREE(startData);

	return true;
}


/******************************************************************************************/



/**************************************** 传感数据采集 *****************************************/

/* 更新单节电芯电压 250ms更新一次 */
void BQ769X0_UpdateCellVolt(void)
{
	uint8_t index = 0;
	uint16_t iTemp = 0;
	uint8_t *pRawADCData = NULL;	
	uint32_t lTemp = 0;

 	if (BQ769X0_ReadBlockWithCRC(VC1_HI_BYTE, &(Registers.VCell1.VCell1Byte.VC1_HI), BQ769X0_CELL_MAX << 1) != true)
 	{
		BQ769X0_ERROR("Update Cell Voltage Fail");
 	}
	

	pRawADCData = &Registers.VCell1.VCell1Byte.VC1_HI;
	for (index = 0; index < BQ769X0_CELL_MAX; index++)
	{
		iTemp = (unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1);
		lTemp = ((unsigned long)iTemp * iGain) / 1000;
		lTemp += Adcoffset;
		BQ769X0_SampleData.CellVoltage[index] = lTemp / 1000.0;
		pRawADCData += 2;
	}
}


/* 热敏电阻温度 2s更新一次 */
void BQ769X0_UpdateTsTemp(void)
{
	uint8_t index;
	uint16_t iTemp = 0;
	uint32_t v_tsx = 0;
	uint8_t *pRawADCData = NULL;

	if (TempSampleMode != 0)
	{
		TempSampleMode = 0;
		if (BQ769X0_WriteRegisterByteWithCRC(SYS_CTRL1, 0x18) != true)
	 	{
			BQ769X0_ERROR("Update Tsx Temperature Fail");
	 	}
		BQ769X0_DELAY(2000);
	}

	if (BQ769X0_ReadBlockWithCRC(TS1_HI_BYTE, &(Registers.TS1.TS1Byte.TS1_HI), BQ769X0_TMEP_MAX << 1) != true)
 	{
		BQ769X0_ERROR("Update Tsx Temperature Fail");
 	}

	pRawADCData = &Registers.TS1.TS1Byte.TS1_HI;
	for(index = 0; index < BQ769X0_TMEP_MAX; index++)
	{
		iTemp = (uint16_t)(*pRawADCData << 8) | *(pRawADCData + 1);
		v_tsx = (uint32_t)iTemp * 382 / 1000;
		BQ769X0_SampleData.TsxTemperature[index] = TempChange(v_tsx) / 10.0;	/*现在是电阻值，需要根据NTC电阻的数据表查表得到对应的温度值*/
		pRawADCData += 2;
	}
}



/* 获取ic内部温度,2s更新一次,未测试好 */
void BQ769X0_UpdateDieTemp(void)
{
	uint16_t adc_value = 0;
	
	if (TempSampleMode != 1)
	{
		TempSampleMode = 1;
		if (BQ769X0_WriteRegisterByteWithCRC(SYS_CTRL1, 0x10) != true)
	 	{
			BQ769X0_ERROR("Update Die Temperature Fail");
	 	}
		BQ769X0_DELAY(2000);
	}

	if (BQ769X0_ReadRegisterWordWithCRC(TS1_HI_BYTE, &Registers.TS1.TS1Word) != true)
 	{
		BQ769X0_ERROR("Update Die Temperature Fail");
 	}
	
	adc_value = (Registers.TS1.TS1Byte.TS1_HI << 8) | Registers.TS1.TS1Byte.TS1_LO;
	BQ769X0_SampleData.DieTemperature =  adc_value * 382.0 / 1000.0;
	BQ769X0_SampleData.DieTemperature = 25 - ((BQ769X0_SampleData.DieTemperature - 1.2) / 0.0042);
}


/* 更新总电流 250ms更新一次 */
void BQ769X0_UpdateCurrent(void)
{
	int32_t temp;

	if (BQ769X0_ReadRegisterWordWithCRC(CC_HI_BYTE, &Registers.CC.CCWord) != true)
 	{
		BQ769X0_ERROR("Update Current Fail");
 	}
	
	temp = Registers.CC.CCByte.CC_HI << 8 | Registers.CC.CCByte.CC_LO;

	//BQ769X0_INFO("current = %d", temp);
	/*CC Reading (in μV) = [16-bit 2’s Complement Value] × (8.44 μV/LSB) */
	if(temp & 0x8000)
	{
		temp = -((~temp + 1) & 0xFFFF);
	}
	
	// unit is A
	BQ769X0_SampleData.BatteryCurrent = ((temp * 8.44) / RsnsValue ) * 0.000001;
}


/* 更新总电压 250ms更新一次 */
void BQ769X0_UpadteBatVolt(void)
{
	uint16_t adc_value;

	if (BQ769X0_ReadRegisterWordWithCRC(BAT_HI_BYTE, &Registers.VBat.VBatWord) != true)
 	{
		BQ769X0_ERROR("Update Battery Voltage Fail");
 	}

	adc_value = Registers.VBat.VBatByte.BAT_HI << 8 | Registers.VBat.VBatByte.BAT_LO;
	BQ769X0_SampleData.BatteryVoltage = 4 * Gain * adc_value;
	BQ769X0_SampleData.BatteryVoltage += BQ769X0_CELL_MAX * Adcoffset; //unit is mV
	BQ769X0_SampleData.BatteryVoltage /= 1000;
}
/*********************************************************************************************/


// 报警处理
static void BQ769X0_AlertHandler(void)
{
	uint8_t reg_value = 0, write_value = 0;

	BQ769X0_ReadRegisterByteWithCRC(SYS_STAT, &reg_value);
	if (reg_value & SYS_STAT_OCD_BIT)
	{
		write_value |= SYS_STAT_OCD_BIT;
		if (AlertOps.ocd != NULL) AlertOps.ocd();
	}

	if (reg_value & SYS_STAT_SCD_BIT)
	{
		write_value |= SYS_STAT_SCD_BIT;
		if (AlertOps.scd != NULL) AlertOps.scd();
	}

	if (reg_value & SYS_STAT_OV_BIT)
	{
		write_value |= SYS_STAT_OV_BIT;
		if (AlertOps.ov != NULL) AlertOps.ov();
	}	

	if (reg_value & SYS_STAT_UV_BIT)
	{
		write_value |= SYS_STAT_UV_BIT;
		if (AlertOps.uv != NULL) AlertOps.uv();
	}

	if (reg_value & SYS_STAT_OVRD_BIT)
	{
		write_value |= SYS_STAT_OVRD_BIT;
		if (AlertOps.ovrd != NULL) AlertOps.ovrd();
	}	

	if (reg_value & SYS_STAT_DEVICE_BIT)
	{
		write_value |= SYS_STAT_DEVICE_BIT;
		if (AlertOps.device != NULL) AlertOps.device();
	}
	
	if (reg_value & SYS_STAT_CC_BIT)
	{
		write_value |= SYS_STAT_CC_BIT;
		if (AlertOps.cc != NULL) AlertOps.cc();
	}		

	BQ769X0_WriteRegisterByteWithCRC(SYS_STAT, write_value);
}


// 获取增益和偏移量
void BQ769X0_GetADCGainOffset(void)
{
	BQ769X0_ReadRegisterByteWithCRC(ADCGAIN1, &(Registers.ADCGain1.ADCGain1Byte));
	BQ769X0_ReadRegisterByteWithCRC(ADCGAIN2, &(Registers.ADCGain2.ADCGain2Byte));
	BQ769X0_ReadRegisterByteWithCRC(ADCOFFSET, &(Registers.ADCOffset));

	
	/*GAIN is uV/LSB,OFFSET is mV*/
	Gain = (ADCGAIN_BASE + ((Registers.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Registers.ADCGain2.ADCGain2Byte & 0xE0)>> 5)) / 1000.0;
	iGain = ADCGAIN_BASE + ((Registers.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Registers.ADCGain2.ADCGain2Byte & 0xE0)>> 5);


	if (Registers.ADCOffset & 0x80)
	{
		Adcoffset = 256 - (int16_t)Registers.ADCOffset * -1;
	}
	else
	{
		Adcoffset = Registers.ADCOffset;
	}
	
	//BQ769X0_INFO("Adcoffset = %d, Registers.ADCOffset = %d", Adcoffset, Registers.ADCOffset);
}


// 配置寄存器
static void BQ769X0_Configuration(void)
{
	unsigned char ReadBuffer[8];

	// 开ADC,选择外部NTC
	Registers.SysCtrl1.SysCtrl1Byte = 0x18;
	
	// 使能电流连续采样，关闭充放电MOS
	Registers.SysCtrl2.SysCtrl2Byte = 0x40;

	// 配置CC_CFG,说明书要求在初始化时应配置为0X19以获得更好的性能
	Registers.CCCfg = 0x19;

	// 写入配置到寄存器
	BQ769X0_WriteBlockWithCRC(SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 8);
	BQ769X0_ReadBlockWithCRC(SYS_CTRL1, ReadBuffer, 8);
	
	
//	BQ769X0_INFO("Read Compare SysCtrl1Byte 0x%02x 0x%02x", ReadBuffer[0]&0X7F, Registers.SysCtrl1.SysCtrl1Byte);
//	BQ769X0_INFO("Read Compare SysCtrl2Byte 0x%02x 0x%02x", ReadBuffer[1], Registers.SysCtrl2.SysCtrl2Byte);
//	BQ769X0_INFO("Read Compare Protect1Byte 0x%02x 0x%02x", ReadBuffer[2], Registers.Protect1.Protect1Byte);
//	BQ769X0_INFO("Read Compare Protect2Byte 0x%02x 0x%02x", ReadBuffer[3], Registers.Protect2.Protect2Byte);
//	BQ769X0_INFO("Read Compare Protect3Byte 0x%02x 0x%02x", ReadBuffer[4], Registers.Protect3.Protect3Byte);
//	BQ769X0_INFO("Read Compare OVTrip 0x%02x 0x%02x", ReadBuffer[5], Registers.OVTrip);
//	BQ769X0_INFO("Read Compare UVTrip 0x%02x 0x%02x", ReadBuffer[6], Registers.UVTrip);
//	BQ769X0_INFO("Read Compare CCCfg 0x%02x 0x%02x", ReadBuffer[7], Registers.CCCfg);
	
	
	// 去掉BUFF[0]的最高位,防止因为接上了负载使负载检测置位而没通过校验
	if( (ReadBuffer[0]&0X7F) != Registers.SysCtrl1.SysCtrl1Byte
	|| ReadBuffer[1] != Registers.SysCtrl2.SysCtrl2Byte
	|| ReadBuffer[2] != Registers.Protect1.Protect1Byte
	|| ReadBuffer[3] != Registers.Protect2.Protect2Byte
	|| ReadBuffer[4] != Registers.Protect3.Protect3Byte
	|| ReadBuffer[5] != Registers.OVTrip
	|| ReadBuffer[6] != Registers.UVTrip
	|| ReadBuffer[7] != Registers.CCCfg)
	{
		BQ769X0_ERROR("BQ769X0 config register fail,Please reset BMS board");

		while(1);
	}
}



// 检测是否接了负载
// 只有在没使能充电的情况下且CHG引脚电压大于0.7V才会检测到负载
bool BQ769X0_LoadDetect(void)
{
	BQ769X0_ReadRegisterWordWithCRC(SYS_CTRL1, (uint16_t *)&Registers.SysCtrl1.SysCtrl1Byte);
	if (Registers.SysCtrl2.SysCtrl2Bit.CHG_ON == 0) // 不在充电状态下
	{
		if (Registers.SysCtrl1.SysCtrl1Bit.LOAD_PRESENT)
		{
			return true;
		}
	}
	return false;
}

// 唤醒BQ芯片
void BQ769X0_Wakeup(void)
{
    BQ769X0_TS1_SetOutMode();
    HAL_GPIO_WritePin(BQ769X0_TS1_GPIO_Port, BQ769X0_TS1_Pin, GPIO_PIN_SET);
    BQ769X0_DELAY(1000);
    HAL_GPIO_WritePin(BQ769X0_TS1_GPIO_Port, BQ769X0_TS1_Pin, GPIO_PIN_RESET);
    BQ769X0_TS1_SetInMode();  // 设为输入模式，避免干扰温度采样
    BQ769X0_DELAY(1000);
}

// 进入低功率模式
void BQ769X0_EntryShip(void)
{
	BQ769X0_WriteRegisterByteWithCRC(SYS_CTRL1, 0x00);
	BQ769X0_WriteRegisterByteWithCRC(SYS_CTRL1, 0x01);
	BQ769X0_WriteRegisterByteWithCRC(SYS_CTRL1, 0x02);
}

// 控制充放电开关
void BQ769X0_ControlDSGOrCHG(BQ769X0_ControlTypedef ControlType, BQ769X0_StateTypedef NewState)
{
	if (NewState == BQ_STATE_ENABLE)
	{
		Registers.SysCtrl2.SysCtrl2Byte |= ControlType;
	}
	else
	{
		Registers.SysCtrl2.SysCtrl2Byte &= ~ControlType;
	}
	BQ769X0_WriteRegisterByteWithCRC(SYS_CTRL2, Registers.SysCtrl2.SysCtrl2Byte);
}


// 设置某个电芯均衡状态，可以位与多节，支持BQ769X0系列(相邻单元不能同时均衡)
void BQ769X0_CellBalanceControl(BQ769X0_CellIndexTypedef CellIndex, BQ769X0_StateTypedef NewState)
{
	static uint8_t CELL_BAL_VALUE[3] = {0};

	if (NewState == BQ_STATE_ENABLE)
	{
		CELL_BAL_VALUE[0] |= CellIndex & 0x1F;
		CELL_BAL_VALUE[1] |= (CellIndex >> 5) & 0x1F;
		CELL_BAL_VALUE[2] |= (CellIndex >> 10) & 0x1F;
	}
	else if (NewState == BQ_STATE_DISABLE)
	{
		CELL_BAL_VALUE[0] &= ~(CellIndex & 0x1F);
		CELL_BAL_VALUE[1] &= ~((CellIndex >> 5) & 0x1F);
		CELL_BAL_VALUE[2] &= ~((CellIndex >> 10) & 0x1F);
	}
	BQ769X0_WriteBlockWithCRC(CELLBAL1, CELL_BAL_VALUE, 3);
}










void BQ769X0_SCDDelaySet(BQ769X0_SCDDelayTypedef SCDDelay)
{
	Registers.Protect1.Protect1Bit.SCD_DELAY = SCDDelay;
	BQ769X0_WriteRegisterByteWithCRC(PROTECT1, Registers.Protect1.Protect1Bit.SCD_DELAY);
}

void BQ769X0_OCDDelaySet(BQ769X0_OCDDelayTypedef OCDDelay)
{
	Registers.Protect2.Protect2Bit.OCD_DELAY = OCDDelay;
	BQ769X0_WriteRegisterByteWithCRC(PROTECT2, Registers.Protect2.Protect2Bit.OCD_DELAY);
}

void BQ769X0_UVDelaySet(BQ769X0_UVDelayTypedef UVDelay)
{
	Registers.Protect3.Protect3Bit.UV_DELAY = UVDelay;
	BQ769X0_WriteRegisterByteWithCRC(PROTECT3, Registers.Protect3.Protect3Bit.UV_DELAY);
}

void BQ769X0_OVDelaySet(BQ769X0_OVDelayTypedef OVDelay)
{
	Registers.Protect3.Protect3Bit.OV_DELAY = OVDelay;
	BQ769X0_WriteRegisterByteWithCRC(PROTECT3, Registers.Protect3.Protect3Bit.OV_DELAY);
}

void BQ769X0_UVPThresholdSet(uint16_t UVPThreshold)
{
	Registers.UVTrip = (uint8_t)((((uint16_t)((UVPThreshold - Adcoffset)/Gain/* + 0.5*/) - UV_THRESH_BASE) >> 4) & 0xFF);
	BQ769X0_WriteRegisterByteWithCRC(UV_TRIP, Registers.UVTrip);
}

void BQ769X0_OVPThresholdSet(uint16_t OVPThreshold)
{
    Registers.OVTrip = (uint8_t)((((uint16_t)((OVPThreshold - Adcoffset)/Gain/* + 0.5*/) - OV_THRESH_BASE) >> 4) & 0xFF);
	BQ769X0_WriteRegisterByteWithCRC(OV_TRIP, Registers.OVTrip);
}






// BQ芯片初始化
void BQ769X0_Initialize(BQ769X0_InitDataTypedef *InitData)
{
    //BQ76X0_GpioInit();


	// 进入睡眠再唤醒相当于复位一次BQ芯片
	BQ769X0_EntryShip();
	BQ769X0_DELAY(500);
	BQ769X0_Wakeup();


	// 获取增益和偏移量
	BQ769X0_GetADCGainOffset();


	AlertOps = InitData->AlertOps;


	// 配置寄存器
	Registers.Protect1.Protect1Bit.SCD_THRESH = SCDThresh;
	Registers.Protect2.Protect2Bit.OCD_THRESH = OCDThresh;
	Registers.Protect1.Protect1Bit.SCD_DELAY  = InitData->ConfigData.SCDDelay;	
	Registers.Protect2.Protect2Bit.OCD_DELAY  = InitData->ConfigData.OCDDelay;	
	Registers.Protect3.Protect3Bit.UV_DELAY   = InitData->ConfigData.UVDelay;	
	Registers.Protect3.Protect3Bit.OV_DELAY   = InitData->ConfigData.OVDelay;
    Registers.OVTrip = (uint8_t)((((uint16_t)((InitData->ConfigData.OVPThreshold - Adcoffset)/Gain/* + 0.5*/) - OV_THRESH_BASE) >> 4) & 0xFF);
    Registers.UVTrip = (uint8_t)((((uint16_t)((InitData->ConfigData.UVPThreshold - Adcoffset)/Gain/* + 0.5*/) - UV_THRESH_BASE) >> 4) & 0xFF);


    BQ769X0_Configuration();
	
//	BQ769X0_INFO("OVTrip:%d", Registers.OVTrip);
//	BQ769X0_INFO("UVTrip:%d", Registers.UVTrip);
	
	BQ769X0_INFO("BQ769X0 Initialize successful!");
}



