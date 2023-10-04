#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "webserver.h"
#include "wifiap.h"
#include "esp_wifi.h"

void taskSpi( void *arg );
void taskWebserver( void *arg );

void app_main(void)
{
	xTaskCreate(taskSpi, "SPI", 2000, NULL, 2, NULL);
	xTaskCreate(taskWebserver, "web server", 4000, NULL, 1, NULL);



    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}


void taskSpi( void *arg )
{
	while(1)
	{
	printf("SPI\n");
	vTaskDelay(1000/ portTICK_PERIOD_MS);
	}
}

void taskWebserver( void *arg )
{
	wifiap_init(); // init wifi and event handler

//	wifi_country_t country = {
//	    .cc = "01",
//	    .schan = 1,
//	    .nchan = 11,
//	    .policy = WIFI_COUNTRY_POLICY_AUTO,
//	};
//
//	ESP_ERROR_CHECK(esp_wifi_set_country(&country));

	wifi_scan_config_t params;
	memset(&params, 0, sizeof(wifi_scan_config_t));

//	wifi_scan_config_t scanConfig;
//	scanConfig.ssid = NULL;
//	scanConfig.bssid = NULL;
//	scanConfig.channel = 0;
//	scanConfig.show_hidden = false;
//	scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
//	scanConfig.scan_time.active.min = 0;
//	scanConfig.scan_time.active.max = 520;
//	scanConfig.scan_time.passive = 120;


//	webserver_init();
	while (1)
	{
		esp_wifi_scan_start(&params, true);
		uint16_t apCount = 0;
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&apCount));
		printf("%d wifi access points found\n\r", apCount);

		wifi_ap_record_t record[20];
		uint16_t maxAps = 20;
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&maxAps, &record));

		uint8_t n;
		for (n = 0; n < maxAps; n++)
		{
			printf("SSID:%s, RSSI: %d\n\r", record[n].ssid, record[n].rssi);
		}


		printf("WEB\n");
		vTaskDelay(1000/ portTICK_PERIOD_MS);
	}
}
