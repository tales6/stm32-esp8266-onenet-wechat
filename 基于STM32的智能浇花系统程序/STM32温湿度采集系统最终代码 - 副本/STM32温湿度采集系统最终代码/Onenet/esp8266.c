#include "stm32f10x.h"
#include "esp8266.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"YOUR_WIFI_SSID\",\"YOUR_WIFI_PASSWORD\"\r\n"
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"

unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

_Bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0)
		return REV_WAIT;

	if(esp8266_cnt == esp8266_cntPre)
	{
		esp8266_cnt = 0;
		return REV_OK;
	}

	esp8266_cntPre = esp8266_cnt;
	return REV_WAIT;
}

_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;

	Usart_SendString(USART1, (unsigned char *)cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)
			{
				ESP8266_Clear();
				return 0;
			}
		}

		DelayXms(10);
	}

	return 1;
}

void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	char cmdBuf[32];

	ESP8266_Clear();
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);
	if(!ESP8266_SendCmd(cmdBuf, ">"))
	{
		Usart_SendString(USART1, data, len);
	}
}

unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
	char *ptrIPD = NULL;

	do
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			ptrIPD = strstr((char *)esp8266_buf, "+IPD,");
			if(ptrIPD == NULL)
			{
				ptrIPD = strstr((char *)esp8266_buf, "IPD,");
			}
			if(ptrIPD != NULL)
			{
				ptrIPD = strchr(ptrIPD, ':');
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
			}
		}

		DelayXms(5);
		timeOut--;
	} while(timeOut>0);

	return NULL;
}

void ESP8266_Init(void)
{
	ESP8266_Clear();

	while(ESP8266_SendCmd("AT\r\n", "OK"))
		DelayXms(500);

	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		DelayXms(500);

	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		DelayXms(500);

	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		DelayXms(500);

	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		DelayXms(500);
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		if(esp8266_cnt >= sizeof(esp8266_buf)) esp8266_cnt = 0;
		esp8266_buf[esp8266_cnt++] = USART1->DR;

		USART_ClearFlag(USART1, USART_FLAG_RXNE);
	}
}
