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
#include "evse.h"


void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  xTaskCreate(taskDebug, "debug task", 4000, NULL, 5, NULL);
  xTaskCreate(taskApp, "app", 7000, NULL, 3, NULL);
  xTaskCreate(taskUdpDaemon, "udp", 2000, NULL, 1, NULL);
  xTaskCreate(taskEvse, "evse", 4000, NULL, 4, NULL);

  while (1)
  {
    printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}



