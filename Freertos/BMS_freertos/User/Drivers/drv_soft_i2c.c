/**************************************************************************/
/*********** 该i2c驱动从rtt的i2c驱动框架移植而来,剔除了device层 ***********/
/**** 考虑了并发情况,考虑到片内资源单独跑os内核的情况可以用,并不占资源 ****/
/**************************************************************************/




#include "drv_soft_i2c.h"

#include "main.h"


// 使用互斥锁会被高优先级任务抢占
// 偶发性的导致i2c信号传输一半的时候就跑去做其他的
// 最后导致读写i2c数据不对进而BQ芯片驱动的CRC通不过
//static struct rt_mutex mutex1 ={0};

//static rt_uint32_t level;


static void I2C1_LockInit(void)
{
    //rt_mutex_init(&mutex1, "i2c1_lock", RT_IPC_FLAG_FIFO);
}

static void I2C1_Lock(void)
{
    //rt_mutex_take(&mutex1, RT_WAITING_FOREVER);
	taskDISABLE_INTERRUPTS();
}

static void I2C1_Unlock(void)
{
    //rt_mutex_release(&mutex1);

    taskENABLE_INTERRUPTS();
}

// 适用于72MHZ
static void delay_us(uint32_t us)
{
	uint16_t i = 0;
	
	while(us--)
	{
		i = 10; //自己定义
		while(i--);
	}
}

// i2c总线句柄
struct I2C_BusTypeDef i2c1 = 
{
	.gpiox = I2C1_SCL_GPIO_Port,
	.sda_gpio_pin = I2C1_SDA_Pin,
	.scl_gpio_pin = I2C1_SCL_Pin,
	.retries = 3,
	.udelay = (void (*)(uint32_t))delay_us,
	.lockInit = I2C1_LockInit,
	.lock = I2C1_Lock,
	.unlock = I2C1_Unlock,
};


static void I2C_BusHardwareInitialize(struct I2C_BusTypeDef *bus)
{
	GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = bus->scl_gpio_pin | bus->sda_gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(bus->gpiox, &GPIO_InitStruct);

    HAL_GPIO_WritePin(bus->gpiox, bus->scl_gpio_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(bus->gpiox, bus->sda_gpio_pin, GPIO_PIN_SET);
	
    if (bus->lockInit)bus->lockInit();
}


static inline void SDA_SetOutMode(struct I2C_BusTypeDef *bus)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = bus->sda_gpio_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(bus->gpiox, &GPIO_InitStruct);
}

// stm32的IO口结构输出模式下是没有关断输入部分的肖特基触发器,数据依然会读入输入寄存器,故不用设置SDA输入模式
// 但为了保险起见最好还是写上,也为了方便将该驱动移植到其他平台或者HAL库上
static inline void SDA_SetInMode(struct I2C_BusTypeDef *bus)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = bus->sda_gpio_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(bus->gpiox, &GPIO_InitStruct);
}

static inline uint8_t GET_SDA(struct I2C_BusTypeDef *bus)
{
	return HAL_GPIO_ReadPin(bus->gpiox, bus->sda_gpio_pin);
}

static inline void SDA_L(struct I2C_BusTypeDef *bus)
{
    HAL_GPIO_WritePin(bus->gpiox, bus->sda_gpio_pin, GPIO_PIN_RESET);
}

static inline void SDA_H(struct I2C_BusTypeDef *bus)
{
    HAL_GPIO_WritePin(bus->gpiox, bus->sda_gpio_pin, GPIO_PIN_SET);
}

static inline void SCL_L(struct I2C_BusTypeDef *bus)
{
    HAL_GPIO_WritePin(bus->gpiox, bus->scl_gpio_pin, GPIO_PIN_RESET);
}

static inline void SCL_H(struct I2C_BusTypeDef *bus)
{
    HAL_GPIO_WritePin(bus->gpiox, bus->scl_gpio_pin, GPIO_PIN_SET);
}

static inline void I2C_Start(struct I2C_BusTypeDef *bus)
{
	SDA_L(bus);
	bus->udelay(1);
	SCL_L(bus);
}

static inline void I2C_Restart(struct I2C_BusTypeDef *bus)
{
	SDA_H(bus);
	SCL_H(bus);
	bus->udelay(1);
	SDA_L(bus);
	bus->udelay(1);
	SCL_L(bus);
}

static inline void I2C_Stop(struct I2C_BusTypeDef *bus)
{
	SDA_L(bus);
	bus->udelay(1);
	SCL_H(bus);
	bus->udelay(1);
	SDA_H(bus);
	bus->udelay(1);
}

static inline uint8_t I2C_WaitACK(struct I2C_BusTypeDef *bus)
{
	uint8_t ack;

	SDA_H(bus);
	bus->udelay(1);
	SCL_H(bus);
	SDA_SetInMode(bus);
	ack = !GET_SDA(bus);    /* ACK : SDA pin is pulled low */\
	SDA_SetOutMode(bus);
	I2C_INFO("%s", ack ? "ACK" : "NACK");
	SCL_L(bus);

	return ack;
}

static inline void I2C_SendAckOrNack(struct I2C_BusTypeDef *bus, int ack)
{
	if (ack)
		SDA_L(bus);
	bus->udelay(1);
	SCL_H(bus);
	SCL_L(bus);
}

static uint8_t I2C_WriteByte(struct I2C_BusTypeDef *bus, uint8_t data)
{
	uint8_t mask;
	
	for (mask = 0x80; mask != 0; mask >>= 1)
	{
		SCL_L(bus);
		data & mask ? SDA_H(bus) : SDA_L(bus);
		bus->udelay(1);
		SCL_H(bus);
	}
	SCL_L(bus);
	bus->udelay(1);

	return I2C_WaitACK(bus);
}


static uint8_t I2C_ReadByte(struct I2C_BusTypeDef *bus)
{
	uint8_t mask;
	uint8_t data = 0;

	SDA_H(bus);
	bus->udelay(1);
	SDA_SetInMode(bus);
	for (mask = 0x80; mask != 0; mask >>= 1) 
	{
		SCL_H(bus);
		if (GET_SDA(bus)) data |= mask;
		SCL_L(bus);
		bus->udelay(1);
	}
	SDA_SetOutMode(bus);

	return data;
}

static uint16_t I2C_SendBytes(struct I2C_BusTypeDef *bus, struct I2C_MessageTypeDef *msg)
{
	uint8_t ret;
	const uint8_t *ptr = msg->buf;
	uint16_t bytes = 0, count = msg->tLen;
	
	while (count > 0)
	{
		if (msg->flags & I2C_CONTROL_BYTE && I2C_WriteByte(bus, msg->cByte) == 0) // 发送控制字节
		{
			I2C_WARNING("send bytes: NACK.");
			break;
		}

		ret = msg->flags & I2C_SAME_BYTE ? I2C_WriteByte(bus, msg->sByte) : I2C_WriteByte(bus, *ptr) , ptr++;
		
		if ((ret > 0) || (msg->flags & I2C_IGNORE_NACK && (ret == 0)))
		{
			count --;			
			bytes ++;
		}
		else if (ret == 0)
		{
			I2C_WARNING("send bytes: NACK.");
			break;
		}
	}

	return bytes;
}

static uint16_t I2C_RecvBytes(struct I2C_BusTypeDef *bus, struct I2C_MessageTypeDef *msg)
{
	uint8_t val;
	uint8_t *ptr = msg->buf;
	uint16_t bytes = 0, count = msg->tLen;

	while (count > 0)
	{
		val = I2C_ReadByte(bus);
		*ptr = val;
		bytes ++;
		
		ptr++;
		count--;

		I2C_INFO("recieve bytes: 0x%02x, %s",
							val, (msg->flags & I2C_NO_READ_ACK) ?
							"(No ACK/NACK)" : (count ? "ACK" : "NACK"));

		if (!(msg->flags & I2C_NO_READ_ACK))
		{
			I2C_SendAckOrNack(bus, count);
		}
	}

	return bytes;
}

static uint8_t I2C_SendAddress(struct I2C_BusTypeDef *bus, uint8_t addr, uint32_t retries)
{
	uint8_t i, ret = 0;

	for (i = 0; i <= retries; i++)
	{
		ret = I2C_WriteByte(bus, addr);
		if (ret == 1)
		{
			I2C_INFO("response ok.");
			break;
		}
		else if (i == retries)
		{
			I2C_WARNING("no response, please check slave device.");
			break;
		}
		I2C_WARNING("no response, attempt to resend the address. number:%d.", i);
		I2C_Stop(bus);
		bus->udelay(1);
		I2C_Start(bus);
	}

	return ret;
}

static uint8_t I2C_BitSendAddress(struct I2C_BusTypeDef *bus, struct I2C_MessageTypeDef *msg)
{
	uint8_t ret, retries, addr1, addr2;
	uint8_t flags = msg->flags;
	uint8_t ignore_nack = msg->flags & I2C_IGNORE_NACK;


	retries = ignore_nack ? 0 : bus->retries;

	if (flags & I2C_ADDR_10BIT)
	{
		addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
		addr2 = msg->addr & 0xff;

		I2C_INFO("addr1: %d, addr2: %d", addr1, addr2);

		ret = I2C_SendAddress(bus, addr1, retries);
		if ((ret != 1) && !ignore_nack)
		{
			I2C_WARNING("NACK: sending first addr");
			return 0;
		}

		ret = I2C_WriteByte(bus, addr2);
		if ((ret != 1) && !ignore_nack)
		{
			I2C_WARNING("NACK: sending second addr");
			return 0;
		}
		if (flags & I2C_RD)
		{
			I2C_INFO("send repeated start condition");
			I2C_Restart(bus);
			addr1 |= 0x01;
			ret = I2C_SendAddress(bus, addr1, retries);
			if ((ret != 1) && !ignore_nack)
			{
				I2C_ERROR("NACK: sending repeated addr");
				return 0;
			}
		}
	}
	else
	{
		/* 7-bit addr */
		addr1 = msg->addr << 1;
		if (flags & I2C_RD)
				addr1 |= 1;
		ret = I2C_SendAddress(bus, addr1, retries);
		if ((ret != 1) && !ignore_nack)
				return 0;
	}

	return 1;
}

/* 传输消息,如果传输成功返回总的消息数量,失败返回0 */
uint32_t I2C_TransferMessages(struct I2C_BusTypeDef *bus, struct I2C_MessageTypeDef msgs[], uint32_t num)
{
	struct I2C_MessageTypeDef *msg;
	uint32_t i, ret = 0;
	uint8_t ignore_nack;

	if (NULL == bus || NULL == msgs || num == 0)return ret;

	if (bus->lock) bus->lock();

	for (i = 0; i < num; i++)
	{
		msg = &msgs[i];
		ignore_nack = msg->flags & I2C_IGNORE_NACK;
		if (!(msg->flags & I2C_NO_START))
		{
			if (i)
			{
				I2C_Restart(bus);
			}
			else
			{
				I2C_INFO("send start condition");
				I2C_Start(bus);
			}
			ret = I2C_BitSendAddress(bus, msg);
			if ((ret != 1) && !ignore_nack)
			{
				I2C_WARNING("receive NACK from device addr 0x%02x msg %d", msgs[i].addr, i);
				goto out;
			}
		}
		if (msg->flags & I2C_RD)
		{
			ret = I2C_RecvBytes(bus, msg);
			msg->rLen = ret;
			I2C_INFO("read %d byte%s", ret, ret == 1 ? "" : "s");
		}
		else
		{
			ret = I2C_SendBytes(bus, msg);	
			msg->rLen = ret;
			I2C_INFO("write %d byte%s", ret, ret == 1 ? "" : "s");				
			if (msg->rLen != msg->tLen)
			{
				ret = 0;
				goto out;
			}
			
		}
	}
	ret = i;

out:
	if (!(msg->flags & I2C_NO_STOP))
	{
		I2C_INFO("send stop condition");
		I2C_Stop(bus);
	}

	if(bus->unlock) bus->unlock();
	
	return ret;
}


int I2C_BusInitialize(void)
{
	I2C_BusHardwareInitialize(&i2c1);
	
	return 0;
}

