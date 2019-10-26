#ifndef __I2C_H
#define __I2C_H
#include "stm32f10x.h"

void I2C1_Init(void);
void I2C_EE_WaitEepromStandbyState(void);
void I2C_WriteByte(u8 addr, u8 data);
u8 I2C_ReadByte(u8 addr);
void I2C_WriteNumData(u8 addr, u8 *str, u8 numToWrite);
void I2C_ReadNumData(u8 addr, u8 *str, u8 numToRead);

#endif /*__I2C_H */



