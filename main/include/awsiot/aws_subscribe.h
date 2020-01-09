#ifndef __AWS_SUBSCRIBE__
#define __AWS_SUBSCRIBE__
void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData);
void add_relay_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData);
int iot_publish_msg(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, char *msg, int msglen);
#endif
