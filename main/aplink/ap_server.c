/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "softap_prov.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "parameter.h"
#include "aws_interface.h"

#define PORT 3333 //CONFIG_EXAMPLE_PORT

static const char *TAG = "ap-server";
static int ap_task_run;

static void ap_server_task(void *pvParameters)
{
	char rx_buffer[200] = {};
	char addr_str[128] = {};
	char ssid[33] = {};
	char pwd[65] = {};
	char userid[17] = {};
	int addr_family;
	int ip_protocol;
	int listen_sock = -1;
	int sock = -1;
	int ret = -1;
	cJSON *root = NULL;
	cJSON *item = NULL;
	char *relayjson = NULL;

	do
	{
		struct sockaddr_in destAddr;
		destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		destAddr.sin_family = AF_INET;
		destAddr.sin_port = htons(PORT);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
		inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

		listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
		if (listen_sock < 0) 
		{
			ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket created");

		int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
		if (err != 0) 
		{
			ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket binded");

		err = listen(listen_sock, 1);
		if (err != 0) 
		{
			ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket listening");

		struct sockaddr_in sourceAddr;
		uint addrLen = sizeof(sourceAddr);

		while (ap_task_run) 
		{
			int s;
			fd_set rfds;
			struct timeval tv = {
				.tv_sec = 10,
				.tv_usec = 0,
			};
			FD_ZERO(&rfds);
			FD_SET(listen_sock,&rfds);

			s = select(listen_sock + 1, &rfds, NULL, NULL, &tv);
			if(s < 0)
			{
				ESP_LOGE(TAG, "Select failed: errno %d", errno);
				continue;
			}
			else if(s == 0)
			{
				ESP_LOGI(TAG, "Select time out");
				continue;
			}
			else if(s > 0)
			{
				if(FD_ISSET(listen_sock,&rfds))
				{
					memset(&sourceAddr,0,addrLen);
					sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
					if (sock < 0) 
					{
						ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
						continue;
					}
					ESP_LOGI(TAG, "Socket accepted");

					while (1) 
					{
						int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
						// Error occured during receiving
						if (len < 0) 
						{
							ESP_LOGE(TAG, "recv failed: errno %d", errno);
							break;
						}
						// Connection closed
						else if (len == 0) 
						{
							ESP_LOGI(TAG, "Connection closed");
							break;
						}
						// Data received
						else 
						{
							inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

							rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
							ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
							ESP_LOGI(TAG, "%s", rx_buffer);

							do
							{
								root = cJSON_Parse(rx_buffer);
								if(root == NULL)
								{
									printf("json parse error!\n");
									break;
								}
								item = cJSON_GetObjectItemCaseSensitive(root,"Ssid");
								if(cJSON_IsString(item) && (item->valuestring != NULL))
								{
									strcpy(ssid,item->valuestring);
								}
								else
								{
									printf("ssid parse error!\n");
									break;
								}
								item = cJSON_GetObjectItemCaseSensitive(root,"Password");
								if(cJSON_IsString(item) && (item->valuestring != NULL))
								{
									strcpy(pwd,item->valuestring);
								}
								else
								{
									printf("password parse error!\n");
									break;
								}
								item = cJSON_GetObjectItemCaseSensitive(root,"UserId");
								if(cJSON_IsString(item) && (item->valuestring != NULL))
								{
									strcpy(userid,item->valuestring);
								}
								else
								{
									printf("userid parse error!\n");
									break;
								}
								ESP_LOGI(TAG, "ssid=%s,pwd=%s,userid=%s\n",ssid,pwd,userid);
								aws_set_userid(userid);
								ret = app_prov_configure_sta(ssid,pwd);
							}while(0);
							if(root)
							{
								cJSON_Delete(root);
							}
							if(ret == 0)
							{
								relayjson = DumpRelayPara();
								int err = send(sock, relayjson, strlen(relayjson), 0);
								if (err < 0) 
								{
									ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
								}
								free(relayjson);
							}
							else
							{
								int err = send(sock, "{\"ret\":\"error\"}", 15, 0);
								if (err < 0) 
								{
									ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
								}
							}
							break;
						}
					}
				}

				if (sock != -1) 
				{
					ESP_LOGI(TAG, "Shutting down socket and re accepting...");
					shutdown(sock, 0);
					close(sock);
					sock = -1;
				}
			}
		}
	}while(0);

	if (listen_sock != -1) 
	{
		ESP_LOGI(TAG, "Shutting down listen socket and delete task...");
		shutdown(listen_sock, 0);
		close(listen_sock);
	}
	vTaskDelete(NULL);
}

void app_prov_stop_ap_service()
{
	ap_task_run = 0;
}
void app_prov_start_ap_service()
{
	ap_task_run = 1;
    xTaskCreate(ap_server_task, "ap_server", 4096, NULL, 5, NULL);
}
