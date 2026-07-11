#ifndef _ONENET_H_
#define _ONENET_H_

extern uint8_t Humidity_Threshold;

_Bool OneNet_DevLink(void);

void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt);

void OneNet_RevPro(unsigned char *cmd);

void OneNet_Publish(const char *topic, const char *msg);

void OneNet_SendPing(void);

#endif
