#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>

#include "softap_prov.h"

static const char *TAG = "wifilib";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	/* Invoke Provisioning event handler first */
	app_prov_event_handler(ctx, event);

	switch(event->event_id) 
	{
		case SYSTEM_EVENT_AP_START:
			ESP_LOGI(TAG, "SoftAP started");
			break;
		case SYSTEM_EVENT_AP_STOP:
			ESP_LOGI(TAG, "SoftAP stopped");
			break;
		case SYSTEM_EVENT_STA_START:
			ESP_LOGI(TAG, "STA started");
			esp_wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "STA got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			break;
		case SYSTEM_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "SoftAP station:"MACSTR" join, AID=%d",
					MAC2STR(event->event_info.sta_connected.mac),
					event->event_info.sta_connected.aid);
			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "SoftAP station:"MACSTR"leave, AID=%d",
					MAC2STR(event->event_info.sta_disconnected.mac),
					event->event_info.sta_disconnected.aid);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "STA disconnected");
			esp_wifi_connect();
			app_prov_start_softap_provisioning(CONFIG_SOFTAP_SSID, CONFIG_SOFTAP_PASS);
			break;
		default:
			break;
	}
	return ESP_OK;
}

static void wifi_init_sta()
{
    /* Start wifi in station mode with credentials set during provisioning */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start() );
}

void initwifi()
{
    /* Initialize networking stack */
    tcpip_adapter_init();

    /* Set our event handling */
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    /* Check if device is provisioned */
    bool provisioned;
    if (app_prov_is_provisioned(&provisioned) != ESP_OK) 
	{
        ESP_LOGE(TAG, "Error getting device provisioning state");
        return;
    }

    if (provisioned == false) 
	{
        /* If not provisioned, start provisioning via soft AP */
        ESP_LOGI(TAG, "Starting WiFi SoftAP provisioning");
        app_prov_start_softap_provisioning(CONFIG_SOFTAP_SSID, CONFIG_SOFTAP_PASS);
    } 
	else 
	{
        /* Start WiFi station with credentials set during provisioning */
        ESP_LOGI(TAG, "Starting WiFi station");
        wifi_init_sta();
    }
}
