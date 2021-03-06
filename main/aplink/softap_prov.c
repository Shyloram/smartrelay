#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <nvs.h>

#include <task.h>
#include <stdlib.h>
#include <esp_timer.h>
#include <protocomm.h>
#include <protocomm_httpd.h>
#include <protocomm_security0.h>
#include <protocomm_security1.h>
#include <wifi_provisioning/wifi_config.h>
#include "softap_prov.h"

static const char *TAG = "softap_prov";

extern void app_prov_start_ap_service();
extern void app_prov_stop_ap_service();

/**
 * @brief   Data relevant to provisioning application
 */
struct app_prov_data 
{
    esp_timer_handle_t timer;
    wifi_prov_sta_state_t wifi_state;
    wifi_prov_sta_fail_reason_t wifi_disconnect_reason;
};

/* Pointer to provisioning application data */
static struct app_prov_data *g_prov;

/* Task spawned by timer callback */
static void stop_prov_task(void * arg)
{
    ESP_LOGI(TAG, "Stopping provisioning");
	app_prov_stop_ap_service();
    esp_wifi_set_mode(WIFI_MODE_STA);

    /* Timer not needed anymore */
    esp_timer_handle_t timer = g_prov->timer;
    esp_timer_delete(timer);
    g_prov->timer = NULL;

    /* Free provisioning process data */
    free(g_prov);
    g_prov = NULL;
    ESP_LOGI(TAG, "Provisioning stopped");

    vTaskDelete(NULL);
}

/* Callback to be invoked by timer */
static void _stop_prov_cb(void * arg)
{
    xTaskCreate(&stop_prov_task, "stop_prov", 2048, NULL, tskIDLE_PRIORITY, NULL);
}

/* Event handler for starting/stopping provisioning.
 * To be called from within the context of the main
 * event handler.
 */
esp_err_t app_prov_event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    /* If pointer to provisioning application data is NULL
     * then provisioning is not running, therefore return without
     * error */
    if (!g_prov) 
	{
        return ESP_OK;
    }

    switch(event->event_id) 
	{
		case SYSTEM_EVENT_STA_START:
			ESP_LOGI(TAG, "STA Start");
			/* Once configuration is received by tcp server,
			 * device is restarted as both AP and Station.
			 * Once station starts, wait for connection to
			 * establish with configured host SSID and password */
			g_prov->wifi_state = WIFI_PROV_STA_CONNECTING;
			break;

		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "STA Got IP");
			/* Station got IP. That means configuration is successful.
			 * Schedule timer to stop provisioning app after 30 seconds. */
			g_prov->wifi_state = WIFI_PROV_STA_CONNECTED;
			if (g_prov && g_prov->timer) 
			{
				/* Note that, after restarting the WiFi in Station + AP mode, the
				 * user gets disconnected from the AP for a while. But at the same
				 * time, the user app requests for status update from the device
				 * to verify that the provisioning was successful. Therefore, the
				 * turning off of the AP must be delayed long enough for the user
				 * to reconnect and get STA connection status from the device.
				 * Otherwise, the AP will be turned off before the user can
				 * reconnect and thus the user app will see connection timed out,
				 * signaling a failure in provisioning. */
				esp_timer_start_once(g_prov->timer, 1*1000*1000U);
			}
			break;

		case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGE(TAG, "STA Disconnected");
			/* Station couldn't connect to configured host SSID */
			g_prov->wifi_state = WIFI_PROV_STA_DISCONNECTED;
			ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);

			/* Set code corresponding to the reason for disconnection */
			switch (info->disconnected.reason) 
			{
				case WIFI_REASON_AUTH_EXPIRE:
				case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
				case WIFI_REASON_BEACON_TIMEOUT:
				case WIFI_REASON_AUTH_FAIL:
				case WIFI_REASON_ASSOC_FAIL:
				case WIFI_REASON_HANDSHAKE_TIMEOUT:
					ESP_LOGI(TAG, "STA Auth Error");
					g_prov->wifi_disconnect_reason = WIFI_PROV_STA_AUTH_ERROR;
					break;
				case WIFI_REASON_NO_AP_FOUND:
					ESP_LOGI(TAG, "STA AP Not found");
					g_prov->wifi_disconnect_reason = WIFI_PROV_STA_AP_NOT_FOUND;
					break;
				default:
					if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) 
					{
						/*Switch to 802.11 bgn mode */
						esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
					}
					/* If none of the expected reasons,
					 * retry connecting to host SSID */
					g_prov->wifi_state = WIFI_PROV_STA_CONNECTING;
					esp_wifi_connect();
			}
			break;

		default:
			break;
	}
    return ESP_OK;
}

esp_err_t app_prov_is_provisioned(bool *provisioned)
{
    *provisioned = false;

#ifdef CONFIG_RESET_PROVISIONED
    nvs_flash_erase();
#endif

    if (nvs_flash_init() != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to init NVS");
        return ESP_FAIL;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to init wifi");
        return ESP_FAIL;
    }

    /* Get WiFi Station configuration */
    wifi_config_t wifi_cfg;
    if (esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_cfg) != ESP_OK) 
	{
        return ESP_FAIL;
    }

    if (strlen((const char*) wifi_cfg.sta.ssid)) 
	{
        *provisioned = true;
        ESP_LOGI(TAG, "Found ssid %s",     (const char*) wifi_cfg.sta.ssid);
        ESP_LOGI(TAG, "Found password %s", (const char*) wifi_cfg.sta.password);
    }
    return ESP_OK;
}

esp_err_t app_prov_configure_sta(const char* ssid, const char* pwd)
{
	wifi_config_t wifi_cfg = {};

	memcpy((char *) wifi_cfg.sta.ssid, ssid, strnlen(ssid, sizeof(wifi_cfg.sta.ssid)));
	memcpy((char *) wifi_cfg.sta.password, pwd, strnlen(pwd, sizeof(wifi_cfg.sta.password)));

	ESP_LOGI(TAG, "Found ssid %s",     (const char*) wifi_cfg.sta.ssid);
	ESP_LOGI(TAG, "Found password %s", (const char*) wifi_cfg.sta.password);
	/* Configure WiFi as both AP and Station */
	if (esp_wifi_set_mode(WIFI_MODE_APSTA) != ESP_OK) 
	{
		ESP_LOGE(TAG, "Failed to set WiFi mode");
		return ESP_FAIL;
	}
	/* Configure WiFi station with host credentials
	 * provided during provisioning */
	if (esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg) != ESP_OK) 
	{
		ESP_LOGE(TAG, "Failed to set WiFi configuration");
		return ESP_FAIL;
	}
	/* Restart WiFi */
	if (esp_wifi_start() != ESP_OK) 
	{
		ESP_LOGE(TAG, "Failed to restart WiFi");
		return ESP_FAIL;
	}
	/* Connect to AP */
	if (esp_wifi_connect() != ESP_OK) 
	{
		ESP_LOGE(TAG, "Failed to connect WiFi");
		return ESP_FAIL;
	}

	if (g_prov) 
	{
		/* Reset wifi station state for provisioning app */
		g_prov->wifi_state = WIFI_PROV_STA_CONNECTING;
	}
	return ESP_OK;
}

static esp_err_t start_wifi_ap(const char *ssid, const char *pass)
{
    /* Initialize WiFi with default configuration */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to init WiFi : %d", err);
        return err;
    }

    /* Build WiFi configuration for AP mode */
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 5,
        },
    };

    strncpy((char *) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.ssid_len = strlen(ssid);

    if (strlen(pass) == 0) 
	{
        memset(wifi_config.ap.password, 0, sizeof(wifi_config.ap.password));
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    } 
	else 
	{
        strncpy((char *) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

    /* Start WiFi in AP mode with configuration built above */
    err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (err != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to set WiFi mode : %d", err);
        return err;
    }
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (err != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to set WiFi config : %d", err);
        return err;
    }
    err = esp_wifi_start();
    if (err != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to start WiFi : %d", err);
        return err;
    }

    return ESP_OK;
}

esp_err_t app_prov_start_softap_provisioning(const char *ssid, const char *pass)
{
    /* If provisioning app data present,
     * means provisioning app is already running */
    if (g_prov) 
	{
        ESP_LOGI(TAG, "Invalid provisioning state");
        return ESP_FAIL;
    }

    /* Allocate memory for provisioning app data */
    g_prov = (struct app_prov_data *) calloc(1, sizeof(struct app_prov_data));
    if (!g_prov) 
	{
        ESP_LOGI(TAG, "Unable to allocate prov data");
        return ESP_ERR_NO_MEM;
    }

    /* Create timer object as a member of app data */
    esp_timer_create_args_t timer_conf = {
        .callback = _stop_prov_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "stop_softap_tm"
    };
    esp_err_t err = esp_timer_create(&timer_conf, &g_prov->timer);
    if (err != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to create timer");
        return err;
    }

    /* Start WiFi softAP with specified ssid and password */
    err = start_wifi_ap(ssid, pass);
    if (err != ESP_OK) 
	{
        ESP_LOGE(TAG, "Failed to start WiFi AP");
        return err;
    }

    /* Start provisioning service through HTTP */
    app_prov_start_ap_service();

    ESP_LOGI(TAG, "SoftAP Provisioning started with SSID '%s', Password '%s'", ssid, pass);
    return ESP_OK;
}
