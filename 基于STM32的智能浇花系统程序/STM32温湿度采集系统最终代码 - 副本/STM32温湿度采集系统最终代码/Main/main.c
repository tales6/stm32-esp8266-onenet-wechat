#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"
#include "bsp_oled.h"
#include "bsp_key.h"
#include "esp8266.h"
#include "onenet.h"
#include "bsp_Alarm.h"

char PUBLIS_BUF[256];
const char devPubTopic[] = "$sys/YOUR_PRODUCT_ID/YOUR_DEVICE_NAME/thing/property/post";
const char *devSubTopic[] = {"$sys/YOUR_PRODUCT_ID/YOUR_DEVICE_NAME/thing/property/set"};
static uint16_t TimeCount = 0;
static uint16_t PingCount = 0;
uint8_t Alarm_flag = 0;
uint8_t Humidity_Threshold = 50;

static uint8_t setting_mode = 0;
static uint8_t motor_running = 0;
static uint16_t motor_timer = 0;
static uint8_t last_water_alarm = 0;

extern uint16_t ADC_ConvertedValue[1];

#define ADC_DRY_VALUE   0
#define ADC_WET_VALUE   2160

#define WATER_SENSOR_PORT  GPIOA
#define WATER_SENSOR_PIN   GPIO_Pin_2
#define WATER_SENSOR_CLK   RCC_APB2Periph_GPIOA
#define Water_Alarm()      GPIO_ReadInputDataBit(WATER_SENSOR_PORT, WATER_SENSOR_PIN)

uint16_t Read_ADC(void)
{
    return ADC_ConvertedValue[0];
}

float ADC_To_Humidity(uint16_t adc_val)
{
    float range;
    float percent;

    range = (float)(ADC_WET_VALUE - ADC_DRY_VALUE);
    if (range <= 0) return 0.0f;

    if (adc_val <= ADC_DRY_VALUE) return 0.0f;
    if (adc_val >= ADC_WET_VALUE) return 100.0f;

    percent = (float)(adc_val - ADC_DRY_VALUE) / range * 100.0f;
    return percent;
}

#define FLASH_SAVE_ADDR    ((uint32_t)0x0800FC00)
#define FLASH_MAGIC        0xA5A5

static void Threshold_Save(uint8_t val)
{
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_SAVE_ADDR);
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR, FLASH_MAGIC);
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + 2, val);
    FLASH_Lock();
}

static uint8_t Threshold_Load(void)
{
    uint16_t magic = *(volatile uint16_t *)FLASH_SAVE_ADDR;
    uint16_t val = *(volatile uint16_t *)(FLASH_SAVE_ADDR + 2);

    if (magic == FLASH_MAGIC && val > 0 && val <= 100)
        return (uint8_t)val;
    return 50;
}

static void Motor_Control_Check(float humidity)
{
	if(!motor_running)
	{
		if(Humidity_Threshold > (uint8_t)humidity)
		{
			Motor_ON();
			motor_running = 1;
			motor_timer = 0;
		}
	}
	else
	{
		if(++motor_timer >= 400)
		{
			Motor_OFF();
			motor_running = 0;
			motor_timer = 0;
		}
	}
}

static void OLED_Display_Normal(float humidity, uint8_t threshold)
{
    uint8_t i;
    char buf[16];
    uint16_t h_int;

    h_int = (uint16_t)(humidity * 10 + 0.5f);

    for (i = 0; "Humidity:"[i]; i++)
        OLED_ShowChar(i * 8, 0, "Humidity:"[i]);

    sprintf(buf, "%d.%d%%  ", h_int / 10, h_int % 10);
    for (i = 0; buf[i]; i++)
        OLED_ShowChar(72 + i * 8, 0, buf[i]);

    for (i = 0; "Threshold:"[i]; i++)
        OLED_ShowChar(i * 8, 2, "Threshold:"[i]);

    sprintf(buf, "%d %%  ", threshold);
    for (i = 0; buf[i]; i++)
        OLED_ShowChar(80 + i * 8, 2, buf[i]);
}

static void OLED_Display_Setting(uint8_t threshold)
{
    uint8_t i;
    char buf[16];

    for (i = 0; " EDIT THRESHOLD "[i]; i++)
        OLED_ShowChar(i * 8, 0, " EDIT THRESHOLD "[i]);

    sprintf(buf, "    [ %d ] %%  ", threshold);
    for (i = 0; buf[i]; i++)
        OLED_ShowChar(i * 8, 2, buf[i]);

    for (i = 0; " +KEY2  -KEY3  "[i]; i++)
        OLED_ShowChar(i * 8, 4, " +KEY2  -KEY3  "[i]);

    for (i = 0; "K1:exit K4:load"[i]; i++)
        OLED_ShowChar(i * 8, 6, "K1:exit K4:load"[i]);
}

static void OLED_ShowMsg(const char *line1, const char *line2)
{
    uint8_t i;
    OLED_Clear();
    for (i = 0; line1[i]; i++)
        OLED_ShowChar(i * 8, 0, line1[i]);
    if (line2)
        for (i = 0; line2[i]; i++)
            OLED_ShowChar(i * 8, 2, line2[i]);
    OLED_Refresh();
}

static void OLED_Display(float humidity, uint8_t threshold)
{
    OLED_Clear();
    if (setting_mode)
        OLED_Display_Setting(threshold);
    else
        OLED_Display_Normal(humidity, threshold);
    OLED_Refresh();
}

static void Handle_Keys(float humidity)
{
    uint8_t key = Key_Scan(0);

    switch(key)
    {
        case 1:
            setting_mode = !setting_mode;
            if (!setting_mode)
            {
                Threshold_Save(Humidity_Threshold);
                memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
                sprintf(PUBLIS_BUF, "{\"id\":\"123\",\"params\":{\"Threshold\":{\"value\":%d}}}", Humidity_Threshold);
                OneNet_Publish(devPubTopic, PUBLIS_BUF);
            }
            OLED_Display(humidity, Humidity_Threshold);
            break;
        case 2:
            if (setting_mode && Humidity_Threshold < 100)
            {
                Humidity_Threshold += 5;
                if (Humidity_Threshold > 100)
                    Humidity_Threshold = 100;
                OLED_Display(humidity, Humidity_Threshold);
            }
            break;
        case 3:
            if (setting_mode && Humidity_Threshold >= 5)
            {
                Humidity_Threshold -= 5;
                OLED_Display(humidity, Humidity_Threshold);
            }
            else if (setting_mode)
            {
                Humidity_Threshold = 0;
                OLED_Display(humidity, Humidity_Threshold);
            }
            break;
        case 4:
            Humidity_Threshold = Threshold_Load();
            OLED_Display(humidity, Humidity_Threshold);
            break;
    }
}

int main(void)
{
    uint16_t adc_val;
    float humidity = 0.0f;
    uint8_t init_retry;
    char buf[16];

    Delay_Init();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    Usart_Init();

    ADCx_Init();

    OLED_Init();
    OLED_ShowMsg("System Starting", NULL);

    Key_Init();

    {
        GPIO_InitTypeDef gis;
        RCC_APB2PeriphClockCmd(WATER_SENSOR_CLK, ENABLE);
        gis.GPIO_Pin = WATER_SENSOR_PIN;
        gis.GPIO_Mode = GPIO_Mode_IPD;
        GPIO_Init(WATER_SENSOR_PORT, &gis);
    }
    Alarm_Init();
    Motor_Init();

    Humidity_Threshold = Threshold_Load();

    sprintf(buf, "Threshold: %d%%", Humidity_Threshold);
    OLED_ShowMsg("Init OK", buf);
    DelayXms(1000);

    OLED_ShowMsg("WiFi Connecting", "...");
    ESP8266_Init();

    init_retry = 0;
    while(OneNet_DevLink())
    {
        init_retry++;
        sprintf(buf, "Retry %d...", init_retry);
        OLED_ShowMsg("WiFi Connecting", buf);
        DelayXms(500);
    }
    OLED_ShowMsg("OneNet Login", "Success!");

    OneNet_Subscribe(devSubTopic, 1);
    OLED_ShowMsg("Subscribe", "Topic OK");
    DelayXms(500);

    OLED_Display(0.0f, Humidity_Threshold);

    while(1)
    {
        unsigned char *dataPtr = ESP8266_GetIPD(5);
        if(dataPtr != NULL)
        {
            uint8_t old_th = Humidity_Threshold;
            OneNet_RevPro(dataPtr);
            if (Humidity_Threshold != old_th)
                Threshold_Save(Humidity_Threshold);
            OLED_Display(humidity, Humidity_Threshold);
        }

        Handle_Keys(humidity);

        {
            uint8_t water_alarm = Water_Alarm();
            if(water_alarm != last_water_alarm)
            {
                last_water_alarm = water_alarm;
                if(water_alarm) { Alarm_ON(); Alarm_flag = 1; }
                else { Alarm_OFF(); Alarm_flag = 0; }
                memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
                sprintf(PUBLIS_BUF, "{\"id\":\"123\",\"params\":{\"alarm\":{\"value\":%s}}}", Alarm_flag ? "true" : "false");
                OneNet_Publish(devPubTopic, PUBLIS_BUF);
            }
        }

        Motor_Control_Check(humidity);

        if(++TimeCount >= 100)
        {
            uint16_t h_int;

            adc_val = Read_ADC();
            humidity = ADC_To_Humidity(adc_val);

            h_int = (uint16_t)(humidity * 10 + 0.5f);
            memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
            sprintf(PUBLIS_BUF, "{\"id\":\"123\",\"params\":{\"Humidity\":{\"value\":%d.%d},\"Threshold\":{\"value\":%d},\"alarm\":{\"value\":%s}}}", h_int / 10, h_int % 10, Humidity_Threshold, Alarm_flag ? "true" : "false");
            OneNet_Publish(devPubTopic, PUBLIS_BUF);

            if (!setting_mode)
                OLED_Display(humidity, Humidity_Threshold);

            TimeCount = 0;
        }

        if(++PingCount >= 3000)
        {
            OneNet_SendPing();
            PingCount = 0;
        }
    }
}
