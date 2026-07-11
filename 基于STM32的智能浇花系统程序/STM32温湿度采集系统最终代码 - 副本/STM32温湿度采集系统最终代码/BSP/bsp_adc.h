#ifndef _BSP_ADC_H
#define _BSP_ADC_H
#include "stm32f10x.h"

#define ADC_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define ADC_CLK                       RCC_APB2Periph_ADC1

#define ADC_GPIO_APBxClock_FUN        RCC_APB2PeriphClockCmd
#define ADC_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define ADC_PORT                      GPIOA

#define NOFCHANEL					 1

#define ADC_PIN1                      GPIO_Pin_1
#define ADC_CHANNEL1                  ADC_Channel_1

#define ADC_x                         ADC1
#define ADC_DMA_CHANNEL               DMA1_Channel1
#define ADC_DMA_CLK                   RCC_AHBPeriph_DMA1

extern uint16_t ADC_ConvertedValue[1];
void ADCx_Init(void);

#endif
