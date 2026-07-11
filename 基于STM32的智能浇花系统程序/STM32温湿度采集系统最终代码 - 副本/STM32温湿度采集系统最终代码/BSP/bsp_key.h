#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "stm32f10x.h"

#define KEY1_PORT_CLK  RCC_APB2Periph_GPIOB
#define KEY1_PORT	     GPIOB
#define KEY1_PORT_PIN  GPIO_Pin_0

#define KEY2_PORT_CLK  RCC_APB2Periph_GPIOA
#define KEY2_PORT	     GPIOA
#define KEY2_PORT_PIN  GPIO_Pin_7

#define KEY3_PORT_CLK  RCC_APB2Periph_GPIOA
#define KEY3_PORT	     GPIOA
#define KEY3_PORT_PIN  GPIO_Pin_3

#define KEY4_PORT_CLK  RCC_APB2Periph_GPIOA
#define KEY4_PORT	     GPIOA
#define KEY4_PORT_PIN  GPIO_Pin_5

#define KEY1    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)
#define KEY2    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)
#define KEY3    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)
#define KEY4    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)

void Key_Init(void);
uint8_t Key_Scan(uint8_t mode);
#endif
