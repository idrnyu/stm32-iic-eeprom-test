#include "stm32f10x.h"
#include "usart.h"
#include "i2c.h"
#include <stdio.h>


int main(void)
{
	u8 a[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 b[255] = {0};
	u8 i = 0;
	USART1_Init();
	printf("¿ªÊ¼²âÊÔ\r\n");

	I2C1_Init();
	// I2C_WriteNumData(60, a, 10);

	I2C_ReadNumData(0, b, 255);
	for (i = 0; i < 255; i++)
	{
	 	printf("%d\t", b[i]);
	}
}
