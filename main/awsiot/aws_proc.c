#include "aws_proc.h"
#include "aws_interface.h"
#include "aws_subscribe.h"
#include "parameter.h"

static const char *TAG = "aws_prov";

extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;

static char userid[17];
static int set_userid;

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

void aws_set_userid(char* puserid)
{
	set_userid = 1;
	strncpy(userid,puserid,sizeof(userid));
}

char* aws_get_userid()
{
	return userid;
}

static void sync_time()
{
	time_t now = 0;
	struct tm timeinfo ={0};
	char strftime_buf[64];
	int retry = 0;
	const int retry_count = 10;

	ESP_LOGI(TAG, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_init();

	// wait for time to be set
	while (timeinfo.tm_year < (2019 - 1900) && ++retry < retry_count)
	{
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		time(&now);
		localtime_r(&now, &timeinfo);
	}

	// Set timezone to China Standard Time
	setenv("TZ", "CST-8", 1);
	tzset();

	time(&now);
	localtime_r(&now, &timeinfo);

	if (timeinfo.tm_year < (2016 - 1900))
	{
		ESP_LOGE(TAG, "The current date/time error");
	}
	else
	{
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
		ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
	}
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) 
{
    ESP_LOGW(TAG, "MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) 
	{
        return;
    }

    if(aws_iot_is_autoreconnect_enabled(pClient)) 
	{
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    } 
	else 
	{
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) 
		{
            ESP_LOGW(TAG, "Manual Reconnect Successful");
        } 
		else 
		{
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
        }
    }
}

void aws_iot_task(void *param) 
{
	IoT_Error_t rc = FAILURE;
	char HostAddress[255] = AWS_IOT_MQTT_HOST;
	uint32_t port = AWS_IOT_MQTT_PORT;
	char *clientid = NULL;

	AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	mqttInitParams.enableAutoReconnect = false; // We enable this later below
	mqttInitParams.pHostURL = HostAddress;
	mqttInitParams.port = port;
	mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
	mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
	mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;
	mqttInitParams.mqttCommandTimeout_ms = 20000;
	mqttInitParams.tlsHandshakeTimeout_ms = 5000;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	rc = aws_iot_mqtt_init(&client, &mqttInitParams);
	if(SUCCESS != rc) 
	{
		ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
		abort();
	}

	/* Wait for WiFI to show as connected */
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

	//sntp
	sync_time();

	clientid = GetRelayId();
	while(!strcmp(clientid,"Defaule"))
	{
		ESP_LOGE(TAG, "client id not set! CAN NOT connect to aws iot server");
		vTaskDelay(1000 / portTICK_RATE_MS);
	}

	connectParams.keepAliveIntervalInSec = 10;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	/* Client ID is set in the menuconfig of the example */
	connectParams.pClientID = clientid;
	connectParams.clientIDLen = (uint16_t) strlen(clientid);
	connectParams.isWillMsgPresent = false;

	ESP_LOGI(TAG, "Connecting to AWS...");
	do 
	{
		rc = aws_iot_mqtt_connect(&client, &connectParams);
		if(SUCCESS != rc) 
		{
			ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	} while(SUCCESS != rc);

	ESP_LOGI(TAG, "Connected ...");
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */

	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) 
	{
		ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
		abort();
	}

	if(set_userid)
	{
		char rTOPIC[50]= {};
		sprintf(rTOPIC,"Rye/relay/%s/add/#",clientid);
		const int rTOPIC_LEN = strlen(rTOPIC);

		rc = aws_iot_mqtt_subscribe(&client, rTOPIC, rTOPIC_LEN, QOS1, add_relay_callback_handler, NULL);
		if(SUCCESS != rc) 
		{
			ESP_LOGE(TAG, "Error subscribing : %d ", rc);
			abort();
		}

		char *TOPIC = "Rye/relay/add";
		int TOPIC_LEN = strlen(TOPIC);
		const char *SEND_FORMAT = "{\"RelayId\":\"%s\",\"UserId\":\"%s\"}";
		char *publish_msg = NULL;
		int msg_len = asprintf(&publish_msg,SEND_FORMAT,clientid,userid);
		if(msg_len < 0)
		{
			ESP_LOGE(TAG, "Failed to allocate memory for GET request buffer");
			abort();
		}
		iot_publish_msg(&client,TOPIC,TOPIC_LEN,publish_msg,msg_len);
		free(publish_msg);
		set_userid = 0;
	}

	ESP_LOGI(TAG, "Subscribing...");
	ESP_LOGI(TAG, "Subscribed...");

	while(1) 
	{

		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(&client, 100);
		if(NETWORK_ATTEMPTING_RECONNECT == rc) 
		{
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}

		ESP_LOGI(TAG, "Stack remaining for task '%s' is %lu bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
		vTaskDelay(1000 / portTICK_RATE_MS);
	}

	ESP_LOGE(TAG, "An error occurred in the main loop.");
	abort();
}

void InitAwsIot()
{
    xTaskCreate(&aws_iot_task, "aws_iot_task", 9216, NULL, 5, NULL);
}
