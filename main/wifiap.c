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
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifiap.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "IQ-home"// CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      "Faseone1" // CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       1 // CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "wifi softAP";
static EventGroupHandle_t wifi_event_group = NULL;
const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_AP_STOPPED_BIT = BIT1;



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
  wifi_mode_t mode;
  ESP_ERROR_CHECK (esp_wifi_get_mode(&mode));
  ESP_LOGI(TAG, "WIFI Event id: %d, mode: %d", event_id, mode);

    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_START)
    {
      xEventGroupClearBits(wifi_event_group, WIFI_AP_STOPPED_BIT);
    }
    else if (event_id == WIFI_EVENT_AP_STOP)
    {
      xEventGroupSetBits(wifi_event_group, WIFI_AP_STOPPED_BIT);
    }
    else if (event_id == WIFI_EVENT_SCAN_DONE)
    {
    	wifi_event_sta_scan_done_t* event = (wifi_event_sta_scan_done_t*) event_data;
        ESP_LOGI(TAG, "status %d, number of AP: %d", event->status, event->number);
    }
    else if (event_id == WIFI_EVENT_STA_START)
    {
      if (mode == WIFI_MODE_STA)
      {
        ESP_LOGI(TAG, "Connected to AP, no IP yet");
        ESP_ERROR_CHECK(esp_wifi_connect());
      }
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
      ESP_LOGI(TAG, "Got IP.");
      if (mode == WIFI_MODE_STA)
      {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
      }
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
      if (mode == WIFI_MODE_STA)
      {
        ESP_LOGI(TAG, "Discxonnected from AP, event set.");
        ESP_ERROR_CHECK(esp_wifi_connect());
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
      }
    }
}

esp_err_t wifiap_startAp(void)
{
  esp_err_t espErr = ESP_FAIL;

  if (wifiap_isWifiUp())
  {
    ESP_LOGI(TAG, "Stopping wifi:");
    ESP_ERROR_CHECK(esp_wifi_stop());
    xEventGroupWaitBits(wifi_event_group, WIFI_AP_STOPPED_BIT, false, true, portMAX_DELAY);
  }

  ESP_ERROR_CHECK (tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));

  tcpip_adapter_ip_info_t ipInfo;
  IP4_ADDR(&ipInfo.ip, 192,168,0,1);
  //ipInfo.ip = 0xC0A80001; // 192.168.0.1
  IP4_ADDR(&ipInfo.gw, 192,168,0,1);
  //ipInfo.gw = 0xC0A80001; // 192.168.0.1
  IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
  // ipInfo.netmask = 0xFFFFFF00; // 255.255.255.0
  ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo));
  ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

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

  ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
  espErr = ESP_OK;

  return espErr;
}

void wifiap_init()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");

    if (wifi_event_group == NULL)
    {
      wifi_event_group = xEventGroupCreate();
    }

    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

}

/*------------------------------------------------------------------------------
*
* check for the current mode of the wifi
*
* @retval true if AP mode
* ----------------------------------------------------------------------------*/
bool wifiap_isApMode(void)
{
  bool apMode = false;

  wifi_mode_t mode;
  if (esp_wifi_get_mode(&mode) == ESP_OK)
  {
    if ((mode == WIFI_MODE_AP) ||
        (mode == WIFI_MODE_APSTA))
    {
      apMode = true;
      ESP_LOGI(TAG, "AP MODE !");
    }
  }


  return apMode;
}


/*------------------------------------------------------------------------------
*
* return true if connected to wifi
*
* @retval result return true if connected to wifi
* ----------------------------------------------------------------------------*/
bool wifiap_isWifiUp(void)
{
  bool result = false;

  uint32_t uxBits;
  uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, 0);

  if (uxBits & WIFI_CONNECTED_BIT)
  {
    result = true;
  }

  return result;
}
/*------------------------------------------------------------------------------
*
* @param[in,out] ssid ssid to connect too
* @param[in,out] password password to use
* @param[in,out] param3 param3_desc
* @param[in,out] param4 param4_desc
*
* @retval espError return 0 on success, otherwise some ESP_ERROR
* ----------------------------------------------------------------------------*/
esp_err_t wifiap_connectToAp(char* ssid, char* password)
{
  esp_err_t espErr = ESP_FAIL;

  if (wifiap_isApMode() == true)
  {
    ESP_LOGI(TAG, "Stopping wifi:");
    ESP_ERROR_CHECK(esp_wifi_stop());
    xEventGroupWaitBits(wifi_event_group, WIFI_AP_STOPPED_BIT, false, true, portMAX_DELAY);
  }

  wifi_config_t wifi_config = {0};

  char* configSsid = (char*)wifi_config.sta.ssid;
  strcpy(configSsid, ssid);
  strcpy((char*)wifi_config.sta.password, password);
  wifi_config.sta.bssid_set = 0;

  ESP_LOGI(TAG, "Starting connect to AP: %s, %s", wifi_config.sta.ssid, wifi_config.sta.password);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  // Wait for connection
  xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
  espErr = ESP_OK;

  ESP_LOGI(TAG, "Connected to AP");





  return espErr;
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
