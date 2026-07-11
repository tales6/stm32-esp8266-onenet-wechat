#include "bsp_key.h"
#include "bsp_delay.h"


void Key_Init(void) 
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin  = KEY2_PORT_PIN | KEY3_PORT_PIN | KEY4_PORT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin  = KEY1_PORT_PIN;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
//按键扫描函数
//返回按键值
//mode:0,不支持连按;1,支持连按;
//0:没有任何按键按下
//1:KEY1按下
//2:KEY2按下
//3:KEY3按下
//4:KEY4按下
//注意:此函数响应优先级,KEY1>KEY2>KEY3>KEY4!!
uint8_t Key_Scan(uint8_t mode)
{	 
	static uint8_t key_up=1;
	if(mode)key_up=1;		  
	if(key_up&&(KEY1==1||KEY2==1||KEY3==1||KEY4==1))
	{
		DelayXms(10);
		key_up=0;
		if(KEY1==1)return 1;
		else if(KEY2==1)return 2;
		else if(KEY3==1)return 3;
		else if(KEY4==1)return 4;
	}
	else if(KEY1==0&&KEY2==0&&KEY3==0&&KEY4==0)key_up=1; 	    
 	return 0;
}
