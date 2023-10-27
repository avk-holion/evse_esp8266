#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "webserver.h"
#include "wifiap.h"

#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "app.h"
#include "debug.h"

void taskSpi( void *arg );
void taskWebserver( void *arg );

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  xTaskCreate(taskSpi, "SPI", 2000, NULL, 2, NULL);
  xTaskCreate(taskWebserver, "web server", 6000, NULL, 1, NULL);
  xTaskCreate(taskDebug, "debug task", 4000, NULL, 3, NULL);
  xTaskCreate(taskApp, "app", 6000, NULL, 3, NULL);



    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}


void taskSpi( void *arg )
{
	while(1)
	{
	printf("SPI\n");
	vTaskDelay(10000/ portTICK_PERIOD_MS);
	}
}

void taskWebserver( void *arg )
{
	/**
	 * start AP
	 */
	wifiap_init(); // init wifi and event handler


	// webserver_init();
	while (1)
	{

		printf("WEB\n");

//        wifi_ap_record_t record[20];
//        uint16_t count;
//        count = wifiap_scan(record, 20);
//        printf("AP count = %d\n\r", count);
		vTaskDelay(100000/ portTICK_PERIOD_MS);
	}
}


