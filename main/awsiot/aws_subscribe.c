#include "aws_proc.h"
#include "aws_subscribe.h"
#include "aws_interface.h"
#include "parameter.h"

static const char *TAG = "aws_subscribe";

//test demo
void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData)
{
	ESP_LOGI(TAG, "Subscribe callback");
	ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
}

void add_relay_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData)
{
	ESP_LOGI(TAG, "Subscribe callback");
	ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
	if(strstr(topicName,"accepted"))
	{
		SetUserId(aws_get_userid());
	}
}

int iot_publish_msg(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, char *msg, int msglen)
{
	if(!pClient || !topicName || !msg)
	{
		printf("input is null\n");
		return -1;
	}
	printf("msg:%s\n",msg);

	int ret;
	IoT_Publish_Message_Params paramsQOS1;
	paramsQOS1.qos = QOS1;
	paramsQOS1.payload = (void *) msg;
	paramsQOS1.isRetained = 0;
	paramsQOS1.payloadLen = msglen;

	ret = aws_iot_mqtt_publish(pClient, topicName, topicNameLen, &paramsQOS1);
	if (ret == MQTT_REQUEST_TIMEOUT_ERROR)
	{
		ESP_LOGW(TAG, "QOS1 publish ack not received.");
		ret = SUCCESS;
	}
	return ret;
}


