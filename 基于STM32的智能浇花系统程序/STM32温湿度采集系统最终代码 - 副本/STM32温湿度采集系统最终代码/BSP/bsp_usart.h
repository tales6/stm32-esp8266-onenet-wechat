#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"

void Usart1_Init(unsigned int baud);

void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);

void Usart_Init(void);
#endif

