#include "bsp_oled.h"
#include "bsp_oledfont.h"

static void IIC_Stop(void);
/*
*********************************************************************************************************
*	函数功能: IIC_GPIO_Config
*	功能说明: GPIO初始化函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(OLED_SCL_RCC|OLED_SDA_RCC, ENABLE);	/* 使GPIO时钟 */
	
	
	GPIO_InitStructure.GPIO_Pin = OLED_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN;
	GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStructure);

	/* 当SCL高电平时,SDA下降沿表示IIC总线启动信号 */
	IIC_Stop();
}
/*
*********************************************************************************************************
*	函数功能: IIC_Delay
*	功能说明: 延时函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void IIC_Delay(void)
{
	uint8_t i;
	for (i = 0; i < 10; i++);
}

/*
*********************************************************************************************************
*	函数功能: IIC_Start
*	功能说明: CPU产生IIC总线启动信号
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_Start(void)
{
	/* 当SCL高电平时,SDA下降沿表示IIC总线启动信号 */
	OLED_SDA_H;
	OLED_SCL_H;
	IIC_Delay();
	OLED_SDA_L;
	IIC_Delay();
	OLED_SCL_L;
	IIC_Delay();
}

/*
*********************************************************************************************************
*	函数功能: IIC_Stop
*	功能说明: CPU产生IIC总线停止信号
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void IIC_Stop(void)
{
	/* 当SCL高电平时,SDA上升沿表示IIC总线停止信号 */
	OLED_SDA_L;
	OLED_SCL_H;
	IIC_Delay();
	OLED_SDA_H;
}

/*
*********************************************************************************************************
*	函数功能: IIC_WaitAck
*	功能说明: CPU产生一个时钟,并读取器件的ACK应答信号
*	形    参: 无
*	返 回 值: 返回0表示正确应答,1表示无器件响应
*********************************************************************************************************
*/
uint8_t IIC_WaitAck(void)
{
	uint8_t re;

	OLED_SDA_H;	/* CPU释放SDA总线 */
	IIC_Delay();
	OLED_SCL_H;	/* CPU拉高SCL = 1, 此时器件会返回ACK应答 */
	IIC_Delay();
	if (OLED_IIC_SDA_READ())	/* CPU读取SDA总线引脚状态 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	OLED_SCL_L;
	IIC_Delay();
	return re;
}
/*
*********************************************************************************************************
*	函数功能: Write_IIC_Byte
*	功能说明: 向IIC总线写入一个字节
*	形    参: _ucByte
*	返 回 值: 无
*********************************************************************************************************
*/
void Write_IIC_Byte(uint8_t _ucByte)
{
 	uint8_t i;
	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++)
	{		
		if (_ucByte & 0x80)
		{
			OLED_SDA_H;
		}
		else
		{
			OLED_SDA_L;
		}
		IIC_Delay();
		OLED_SCL_H;
		IIC_Delay();	
		OLED_SCL_L;
		if (i == 7)
		{
			 OLED_SDA_H; // 释放总线
		}
		_ucByte <<= 1;	/* 移位下一个bit */
		IIC_Delay();
	}
}
/*
*********************************************************************************************************
*	函数功能: Write_IIC_Command
*	功能说明: 向OLED显示屏写入命令
*	形    参: IIC_Command
*	返 回 值: 无
*********************************************************************************************************
*/

void Write_IIC_Command(u8 IIC_Command)
{
	IIC_Start();
	Write_IIC_Byte(OLED_ADDRESS);//OLED地址
	IIC_WaitAck();
	Write_IIC_Byte(0x00);//寄存器地址
	IIC_WaitAck();
	Write_IIC_Byte(IIC_Command);
	IIC_WaitAck();
	IIC_Stop();
}

/*
*********************************************************************************************************
*	函数功能: Write_IIC_Data
*	功能说明: 向OLED显示屏写入数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/

void Write_IIC_Data(u8 IIC_Data)
{
	IIC_Start();
	Write_IIC_Byte(OLED_ADDRESS);//OLED地址
	IIC_WaitAck();
	Write_IIC_Byte(0x40);//寄存器地址
	IIC_WaitAck();
	Write_IIC_Byte(IIC_Data);
	IIC_WaitAck();
	IIC_Stop();
}
/*
*********************************************************************************************************
*	函数功能: OLED_Set_Pos
*	功能说明: 设置起始位置坐标
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
u8 OLED_GRAM[8][128];

void OLED_Set_Pos(u8 x, u8 y) 
{ 
	Write_IIC_Command(0xb0+y);
	Write_IIC_Command(((x&0xf0)>>4)|0x10);
	Write_IIC_Command((x&0x0f)|0x00);
}

/*
*********************************************************************************************************
*	函数功能: OLED_Fill
*	功能说明: OLED全屏填充
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_Fill(u8 fill_Data)
{
	u8 m,n;
	for(m=0;m<8;m++)
		for(n=0;n<128;n++)
			OLED_GRAM[m][n] = fill_Data;
}

/*
*********************************************************************************************************
*	函数功能: OLED_Clear
*	功能说明: 清空显存
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_Clear(void)
{
	OLED_Fill(0x00);
}
/*
*********************************************************************************************************
*	函数功能: OLED_ShowChar
*	功能说明: 在指定位置显示一个字符
*	形    参: OLED的像素位置:x:0~127 y:0~63
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_ShowChar(u8 x, u8 y, u8 chr)
{
	u8 c, i;
	c = chr - ' ';
	if(x > 127) return;
	if(y > 6) return;
	for(i = 0; i < 8; i++)
	{
		if(x + i < 128)
		{
			OLED_GRAM[y][x + i] = zf[c * 16 + i];
			OLED_GRAM[y + 1][x + i] = zf[c * 16 + i + 8];
		}
	}
}
/*
*********************************************************************************************************
*	函数功能: OLED_Init
*	功能说明: OELD初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void OLED_InitController(void)
{
	Write_IIC_Command(0x00);
	Write_IIC_Command(0x10);
	Write_IIC_Command(0x40);
	Write_IIC_Command(0xB0);
	Write_IIC_Command(0x81);
	Write_IIC_Command(0xFF);
	Write_IIC_Command(0xA1);
	Write_IIC_Command(0xA6);
	Write_IIC_Command(0xA8);
	Write_IIC_Command(0x3F);
	Write_IIC_Command(0xC8);
	Write_IIC_Command(0xD3);
	Write_IIC_Command(0x00);
	Write_IIC_Command(0xD5);
	Write_IIC_Command(0x80);
	Write_IIC_Command(0xD8);
	Write_IIC_Command(0x05);
	Write_IIC_Command(0xD9);
	Write_IIC_Command(0xF1);
	Write_IIC_Command(0xDA);
	Write_IIC_Command(0x12);
	Write_IIC_Command(0xDB);
	Write_IIC_Command(0x30);
	Write_IIC_Command(0x8D);
	Write_IIC_Command(0x14);
}

static void OLED_FlushPage(u8 page)
{
	u8 col;
	OLED_Set_Pos(0, page);
	IIC_Start();
	Write_IIC_Byte(OLED_ADDRESS);
	IIC_WaitAck();
	Write_IIC_Byte(0x40);
	IIC_WaitAck();
	for(col = 0; col < 128; col++)
	{
		Write_IIC_Byte(OLED_GRAM[page][col]);
		IIC_WaitAck();
	}
	IIC_Stop();
}

void OLED_Refresh(void)
{
	u8 page;
	OLED_InitController();
	for(page = 0; page < 8; page++)
		OLED_FlushPage(page);
	Write_IIC_Command(0xAF);
}

void OLED_Init(void)
{
	IIC_GPIO_Config();
	OLED_Clear();
	OLED_Refresh();
}

