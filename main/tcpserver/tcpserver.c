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

#define PORT 3334 //CONFIG_EXAMPLE_PORT

static const char *TAG = "tcp-server";
extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;

static void tcp_server_task(void *pvParameters)
{
	char rx_buffer[1024] = {};
	char addr_str[128] = {};
	char *response = NULL;
	int addr_family;
	int ip_protocol;
	int listen_sock = -1;
	int sock = -1;

	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,false, true, portMAX_DELAY);
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

		while (1) 
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
				//ESP_LOGI(TAG, "Select time out");
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
							response = SetPara(rx_buffer);
							if(response)
							{
								send(sock,response,strlen(response),0);
								free(response);
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

void InitTcpService()
{
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
}
