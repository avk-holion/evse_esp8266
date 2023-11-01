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
#include "udpDaemon.h"

void taskSpi( void *arg );
void taskWebserver( void *arg );

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  xTaskCreate(taskSpi, "SPI", 2000, NULL, 2, NULL);
  xTaskCreate(taskDebug, "debug task", 4000, NULL, 3, NULL);
  xTaskCreate(taskApp, "app", 7000, NULL, 3, NULL);
  xTaskCreate(taskUdpDaemon, "udp", 2000, NULL, 1, NULL);

  while (1)
  {
    printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}


void taskSpi( void *arg )
{
	while(1)
	{
      printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
      vTaskDelay(10000/ portTICK_PERIOD_MS);
	}
}


