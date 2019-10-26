#include "i2c.h"
#include "stm32f10x.h"
#include "usart.h"
#include <stdio.h>

u16 timeout = 1000;

/*i2c ��ʼ�� */
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

/*�ȴ�eeprom�ͷ����� */
void I2C_EE_WaitEepromStandbyState(void)
{
  do
  {
    I2C_GenerateSTART(I2C1, ENABLE);
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_SB) == ERROR); //���EV5�¼�
		I2C_Send7bitAddress(I2C1,0xA1,I2C_Direction_Transmitter);
  } while (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == ERROR);
  I2C_ClearFlag(I2C1, I2C_FLAG_AF); //�����־λ
  I2C_GenerateSTOP(I2C1, ENABLE);   //�����ź�
}

/*д��һ���ֽ�*/
void I2C_WriteByte(u8 addr, u8 data)
{
  I2C_EE_WaitEepromStandbyState();
  I2C_GenerateSTART(I2C1, ENABLE);                            //������ʼ�ź�
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) //���	EV5�¼�
  {
    if ((timeout--) == 0)
      printf("EV5 Fail");
  }
  timeout = 1000;

  I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter);               //����7λEEPROM��Ӳ����ַ
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) //���EV6�¼�
  {
    if ((timeout--) == 0)
      printf("EV6 Fail");
  }
  timeout = 1000;

  I2C_SendData(I2C1, addr);                                        //���Ͳ������ڴ��ַ
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) //���EV8�¼�
  {
    if ((timeout--) == 0)
      printf("EV8 Fail");
  }
  timeout = 1000;

  I2C_SendData(I2C1, data);                                        //Ҫд������ݣ�һ���ֽڣ�
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) //���EV8�¼�
  {
    if ((timeout--) == 0)
      printf("EV8 Fail");
  }
  timeout = 1000;

  I2C_GenerateSTOP(I2C1, ENABLE); //���ͽ����ź�
}

/*��ȡһ���ֽ�*/
u8 I2C_ReadByte(u8 addr)
{
  u8 readtemp;
  I2C_EE_WaitEepromStandbyState(); //�ȴ�EEPROM�ͷ�����

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

  /* ��� EV7 �¼� */
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS)
  {
    if ((timeout--) == 0)
      printf("EV7 Fail");
  }
  timeout = 1000;

  /* ��ȡ�������� */
  readtemp = I2C_ReceiveData(I2C1);

  /* ֹͣ�ź� */
  I2C_GenerateSTOP(I2C1, ENABLE);

  return readtemp;
}

/*	��eeprom��д��n������*/
void I2C_WriteNumData(u8 addr, u8 *str, u8 numToWrite)
{
  if ((addr + numToWrite) > 255) //����д������ݳ����ܵ��ڴ�
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


