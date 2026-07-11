#ifndef _BSP_ALARM_H
#define _BSP_ALARM_H

#include "stm32f10x.h"

#define FMQ_GPIO_PORT    	GPIOB			            /* GPIO port */
#define FMQ_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO clock */
#define FMQ_GPIO_PIN			GPIO_Pin_8			        /* GPIO pin */

#define LED_GPIO_PORT			GPIOB
#define LED_GPIO_CLK  		RCC_APB2Periph_GPIOB
#define LED_GPIO_PIN  		GPIO_Pin_9

void Alarm_Init(void);
void Alarm_OFF(void);
void Alarm_ON(void);

#define MOTOR_GPIO_PORT    GPIOB
#define MOTOR_GPIO_CLK     RCC_APB2Periph_GPIOB
#define MOTOR_GPIO_PIN     GPIO_Pin_3

void Motor_Init(void);
void Motor_ON(void);
void Motor_OFF(void);

#endif
