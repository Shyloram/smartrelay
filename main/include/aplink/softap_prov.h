#ifndef __SOFTAP_PROV__
#define __SOFTAP_PROV__
#pragma once

#include <esp_event_loop.h>
#include <protocomm_security.h>
#include <wifi_provisioning/wifi_config.h>

/**
 * @brief   Event handler for provisioning app
 *
 * This is called from the main event handler and controls the
 * provisioning application, depeding on WiFi events
 *
 * @param[in] ctx   Event context data
 * @param[in] event Event info
 *
 * @return
 *  - ESP_OK      : Event handled successfully
 *  - ESP_FAIL    : Failed to start server on event AP start
 */
esp_err_t app_prov_event_handler(void *ctx, system_event_t *event);

/**
 * @brief   Checks if device is provisioned
 * *
 * @param[out] provisioned  True if provisioned, else false
 *
 * @return
 *  - ESP_OK      : Retrieved provision state successfully
 *  - ESP_FAIL    : Failed to retrieve provision state
 */
esp_err_t app_prov_is_provisioned(bool *provisioned);

/**
 * @brief   Runs WiFi as both AP and Station
 *
 * Configures the WiFi station mode to connect to the
 * SSID and password specified in config structure,
 * and restarts WiFi to run as both AP and station
 *
 * @param[in] wifi_cfg  Pointer to WiFi cofiguration structure
 *
 * @return
 *  - ESP_OK      : WiFi configured and restarted successfully
 *  - ESP_FAIL    : Failed to set configuration
 */
esp_err_t app_prov_configure_sta(const char* ssid, const char* pwd);

/**
 * @brief   Start provisioning via softAP
 *
 * Starts the WiFi softAP with specified ssid and pass, provisioning
 * security mode and proof of possession (if any).
 *
 * @param[in] ssid      SSID for SoftAP
 * @param[in] pass      Password for SoftAP
 * @param[in] security  Security mode
 * @param[in] pop       Pointer to proof of possession (NULL if not present)
 *
 * @return
 *  - ESP_OK      : Provisioning started successfully
 *  - ESP_FAIL    : Failed to start
 */
esp_err_t app_prov_start_softap_provisioning(const char *ssid, const char *pass);
#endif 
