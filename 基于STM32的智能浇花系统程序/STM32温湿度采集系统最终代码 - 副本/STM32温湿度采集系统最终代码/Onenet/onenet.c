#include "stm32f10x.h"
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include "bsp_usart.h"
#include "bsp_delay.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"

#define PROID		"YOUR_PRODUCT_ID"
#define TOKEN		"YOUR_ONENET_TOKEN"
#define DEVID		"YOUR_DEVICE_NAME"

extern unsigned char esp8266_buf[512];
extern uint8_t Alarm_flag;
extern uint8_t Humidity_Threshold;
extern char PUBLIS_BUF[256];

static void MQTT_SendPublish(const char *topic, const char *payload)
{
    uint8_t buf[300];
    uint16_t topic_len = strlen(topic);
    uint16_t payload_len = strlen(payload);
    uint16_t idx = 0;
    uint8_t remaining_len;

    remaining_len = topic_len + payload_len + 2;

    buf[idx++] = 0x30;
    buf[idx++] = remaining_len;

    buf[idx++] = (topic_len >> 8) & 0xFF;
    buf[idx++] = topic_len & 0xFF;
    memcpy(&buf[idx], topic, topic_len);
    idx += topic_len;

    memcpy(&buf[idx], payload, payload_len);
    idx += payload_len;

    ESP8266_SendData(buf, idx);
}

_Bool OneNet_DevLink(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	unsigned char *dataPtr;
	_Bool status = 1;

	if(MQTT_PacketConnect(PROID, TOKEN, DEVID, 256, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		dataPtr = ESP8266_GetIPD(250);

		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				if(MQTT_UnPacketConnectAck(dataPtr) == 0)
					status = 0;
			}
		}

		MQTT_DeleteBuffer(&mqttPacket);
	}

	return status;
}

void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL1, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		MQTT_DeleteBuffer(&mqttPacket);

		ESP8266_GetIPD(100);
	}
}

void OneNet_SendPing(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

	if(MQTT_PacketPing(&mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		MQTT_DeleteBuffer(&mqttPacket);
	}
}

void OneNet_Publish(const char *topic, const char *msg)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		MQTT_DeleteBuffer(&mqttPacket);
	}
}

void OneNet_RevPro(unsigned char *cmd)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	short result = 0;
	cJSON *json, *params_json, *Threshold_json, *id_json;

	type = MQTT_UnPacketRecv(cmd);

	switch(type)
	{
		case MQTT_PKT_PUBLISH:
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				if(pkt_id > 0)
				{
					if(MQTT_PacketPublishAck(pkt_id, &mqttPacket) == 0)
					{
						ESP8266_SendData(mqttPacket._data, mqttPacket._len);
						MQTT_DeleteBuffer(&mqttPacket);
					}
				}

				json = cJSON_Parse(req_payload);
				if(json != NULL)
				{
					id_json = cJSON_GetObjectItem(json, "id");
					if(id_json != NULL)
					{
						memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
						sprintf(PUBLIS_BUF, "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}", id_json->valuestring);

						ESP8266_Clear();
						DelayXms(100);

						{
							char respTopic[128];
							strcpy(respTopic, "$sys/YOUR_PRODUCT_ID/YOUR_DEVICE_NAME/thing/property/set_reply");
							MQTT_SendPublish(respTopic, PUBLIS_BUF);
							DelayXms(100);
						}
					}

					params_json = cJSON_GetObjectItem(json, "params");
					if(params_json != NULL)
					{
					    Threshold_json = cJSON_GetObjectItem(params_json, "Threshold");
					    if(Threshold_json != NULL)
					    {
					        if(Threshold_json->type == cJSON_Number)
					        {
					            Humidity_Threshold = Threshold_json->valueint;
					        }
					        else
					        {
					            cJSON *value_json = cJSON_GetObjectItem(Threshold_json, "value");
					            if(value_json != NULL && value_json->type == cJSON_Number)
					                Humidity_Threshold = value_json->valueint;
					        }
					    }
					}
					cJSON_Delete(json);
				}
			}
			break;

		case MQTT_PKT_PUBACK:
			break;
		case MQTT_PKT_PUBREC:
			break;
		case MQTT_PKT_PUBREL:
			break;
		case MQTT_PKT_PUBCOMP:
			break;
		case MQTT_PKT_SUBACK:
			break;
		case MQTT_PKT_UNSUBACK:
			break;

		default:
			result = -1;
			break;
	}

	ESP8266_Clear();

	if(type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}
