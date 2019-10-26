#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"

void USART1_Init(void);
void USART1_SendByte(u8 data);
void USART1_SendStr(u8 *str);

#endif /* __USART_H */

