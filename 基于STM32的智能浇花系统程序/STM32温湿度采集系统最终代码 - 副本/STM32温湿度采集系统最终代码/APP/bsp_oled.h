#ifndef __BSP_OLED_H
#define	__BSP_OLED_H

#include "stm32f10x.h"

#define OLED_ADDRESS	0x78 //默认0x78

//定义IIC接口连接的GPIO端口, 用户只需要修改以下即可改变SCL和SDA的引脚
#define OLED_SCL_H   GPIO_SetBits(OLED_SCL_GPIO_PORT,OLED_SCL_PIN)   //SCL置高
#define OLED_SDA_H   GPIO_SetBits(OLED_SDA_GPIO_PORT,OLED_SDA_PIN)   //SDA置高
#define OLED_SCL_L   GPIO_ResetBits(OLED_SCL_GPIO_PORT,OLED_SCL_PIN) //SCL置低
#define OLED_SDA_L   GPIO_ResetBits(OLED_SDA_GPIO_PORT,OLED_SDA_PIN) //SDA置低

//SCL:PB6 , SDA:PB7
#define OLED_SCL_GPIO_PORT	GPIOB			/* GPIO端口 */
#define OLED_SCL_RCC 	      RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define OLED_SCL_PIN		    GPIO_Pin_6			/* 连接到SCL时钟线的GPIO */

#define OLED_SDA_GPIO_PORT	GPIOB			/* GPIO端口 */
#define OLED_SDA_RCC 	      RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define OLED_SDA_PIN		    GPIO_Pin_7			/* 连接到SDA数据线的GPIO */


#define OLED_IIC_SDA_READ()  GPIO_ReadInputDataBit(OLED_SDA_GPIO_PORT, OLED_SDA_PIN)	/* 读SDA引脚状态 */


extern u8 OLED_GRAM[8][128];

void Write_IIC_Command(u8 IIC_Command);
void OLED_Clear(void);
void OLED_ShowChar(u8 x, u8 y, u8 chr);
void OLED_Init(void);
void OLED_Refresh(void);
#endif
