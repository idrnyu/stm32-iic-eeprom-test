#include "i2c.h"
#include "stm32f10x.h"
#include "usart.h"
#include <stdio.h>

u16 timeout = 1000;

/*i2c 初始化 */
void I2C1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  I2C_InitTypeDef I2C_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = 40000;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_OwnAddress1 = 0x55;
  I2C_Init(I2C1, &I2C_InitStructure);

  I2C_Cmd(I2C1, ENABLE);
}

/*等待eeprom释放总线 */
void I2C_EE_WaitEepromStandbyState(void)
{
  do
  {
    I2C_GenerateSTART(I2C1, ENABLE);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_SB) == ERROR); //检测EV5事件
		I2C_Send7bitAddress(I2C1,0xA1,I2C_Direction_Transmitter);
  } while (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == ERROR);
  I2C_ClearFlag(I2C1, I2C_FLAG_AF); //清楚标志位
  I2C_GenerateSTOP(I2C1, ENABLE);   //结束信号
}

/*写入一个字节*/
void I2C_WriteByte(u8 addr, u8 data)
{
  I2C_EE_WaitEepromStandbyState();
  I2C_GenerateSTART(I2C1, ENABLE);                            //发送起始信号
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) //检测	EV5事件
  {
    if ((timeout--) == 0)
      printf("EV5 Fail");
  }
  timeout = 1000;

  I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter);               //发送7位EEPROM的硬件地址
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) //检测EV6事件
  {
    if ((timeout--) == 0)
      printf("EV6 Fail");
  }
  timeout = 1000;

  I2C_SendData(I2C1, addr);                                        //发送操作的内存地址
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) //检测EV8事件
  {
    if ((timeout--) == 0)
      printf("EV8 Fail");
  }
  timeout = 1000;

  I2C_SendData(I2C1, data);                                        //要写入的数据（一个字节）
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) //检测EV8事件
  {
    if ((timeout--) == 0)
      printf("EV8 Fail");
  }
  timeout = 1000;

  I2C_GenerateSTOP(I2C1, ENABLE); //发送结束信号
}

/*读取一个字节*/
u8 I2C_ReadByte(u8 addr)
{
  u8 readtemp;
  I2C_EE_WaitEepromStandbyState(); //等待EEPROM释放总线

  I2C_GenerateSTART(I2C1, ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) //5
  {
    if ((timeout--) == 0)
      printf("EV5 Fail");
  }
  timeout = 1000;

  I2C_Send7bitAddress(I2C1, 0xA1, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) //6
  {
    if ((timeout--) == 0)
      printf("EV6 Fail");
  }
  timeout = 1000;

  I2C_SendData(I2C1, addr);
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS)
  {
    if ((timeout--) == 0)
      printf("EV8 Fail");
  }
  timeout = 1000;

  I2C_GenerateSTART(I2C1, ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
  {
    if ((timeout--) == 0)
      printf("EV5 Fail");
  }
  timeout = 1000;

  I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
  {
    if ((timeout--) == 0)
      printf("EV6 Fail");
  }
  timeout = 1000;

  I2C_AcknowledgeConfig(I2C1, DISABLE);

  /* 检测 EV7 事件 */
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS)
  {
    if ((timeout--) == 0)
      printf("EV7 Fail");
  }
  timeout = 1000;

  /* 读取接收数据 */
  readtemp = I2C_ReceiveData(I2C1);

  /* 停止信号 */
  I2C_GenerateSTOP(I2C1, ENABLE);

  return readtemp;
}

/*	向eeprom中写入n个数据*/
void I2C_WriteNumData(u8 addr, u8 *str, u8 numToWrite)
{
  if ((addr + numToWrite) > 255) //避免写入的数据超过总的内存
  {
    printf("refuse to write\n");
    printf("please enter less than %d char\n", (255 - addr));
  }
  else
  {
    printf("allow to write\n");
    while (numToWrite)
    {
      I2C_WriteByte(addr, *str);
      addr++;
      str++;
      numToWrite--;
    }
    printf("write success\n");
  }
}

void I2C_ReadNumData(u8 addr, u8 *str, u8 numToRead)
{
  while (numToRead)
  {
    *str = I2C_ReadByte(addr);
    addr++;
    str++;
    numToRead--;
  }
}


