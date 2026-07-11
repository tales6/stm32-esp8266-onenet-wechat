#include "bsp_Alarm.h"


void Alarm_Init(void)//蜂鸣器初始化函数
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(FMQ_GPIO_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin= FMQ_GPIO_PIN |LED_GPIO_PIN;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(FMQ_GPIO_PORT,&GPIO_InitStructure);
	
	GPIO_ResetBits(FMQ_GPIO_PORT, FMQ_GPIO_PIN);	
	GPIO_SetBits(LED_GPIO_PORT,LED_GPIO_PIN);		
}

void Alarm_OFF()
{
	GPIO_ResetBits(FMQ_GPIO_PORT, FMQ_GPIO_PIN);	
	GPIO_SetBits(LED_GPIO_PORT,LED_GPIO_PIN);		
}

void Alarm_ON()
{
	GPIO_SetBits(FMQ_GPIO_PORT, FMQ_GPIO_PIN);
	GPIO_ResetBits(LED_GPIO_PORT,LED_GPIO_PIN);		
}

void Motor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(MOTOR_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = MOTOR_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_GPIO_PIN);
}

void Motor_ON(void)
{
	GPIO_SetBits(MOTOR_GPIO_PORT, MOTOR_GPIO_PIN);
}

void Motor_OFF(void)
{
	GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_GPIO_PIN);
}
