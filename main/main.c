#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

extern void InitPara();
extern void InitWifi();
extern void InitAwsIot();
extern void InitTcpService();

void app_main()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ",
            chip_info.cores);
    printf("silicon revision %d, ", chip_info.revision);
    printf("%dMB %s flash\n\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	/* Init Parameter */
	InitPara();

	/* Init Wifi Lib */
	InitWifi();

	/* Init Tcp Server */
	//InitTcpService();

	/* Init AWS IOT */
	InitAwsIot();
}
