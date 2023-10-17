/*
 * wifiap.c
 *
 *  Created on: 4. okt. 2023
 *      Author: kh
 */



/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "IQ-home"// CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      "Faseone1" // CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       1 // CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_SCAN_DONE) {
    	wifi_event_sta_scan_done_t* event = (wifi_event_sta_scan_done_t*) event_data;
        ESP_LOGI(TAG, "status %d, number of AP: %d", event->status, event->number);
    }
}

void wifi_init_softap()
{
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);

    tcpip_adapter_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192,168,0,1);
    //ipInfo.ip = 0xC0A80001; // 192.168.0.1
    IP4_ADDR(&ipInfo.gw, 192,168,0,1);
    //ipInfo.gw = 0xC0A80001; // 192.168.0.1
    IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
    // ipInfo.netmask = 0xFFFFFF00; // 255.255.255.0
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo));
    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);



    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void wifiap_init()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
}

/*------------------------------------------------------------------------------
* Search for WIFI acces points, ands store them into an array.
*
* @param[out] list list of found access points.
* @param[in] length max entries in the list.
*
* @retval apCount number of access point found, and stored in the list.
* ----------------------------------------------------------------------------*/
uint16_t wifiap_scan(wifi_ap_record_t* list, uint16_t length)
{
  uint16_t apCount;

  wifi_scan_config_t params;
  memset(&params, 0, sizeof(wifi_scan_config_t));

  esp_wifi_scan_start(&params, true);

  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&apCount));
  printf("%d wifi access points found\n\r", apCount);
  printf("SSID\t, RSSI\n\r");

  if (apCount > length)
  {
    apCount = length;
  }

  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));

  uint8_t n;
  for (n = 0; n < apCount; n++)
  {
    printf("%s\t, %d\n\r", list[n].ssid, list[n].rssi);
  }

  return apCount;
}
