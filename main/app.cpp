/*------------------------------------------------------------------------------
* app.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 16. okt. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

#include "app.h"
#include "webserver.h"
#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_log.h"
#include "cJSON.h"
#include "wifiap.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/

namespace  // anonymous
{
/*------------------------------------------------------------------------------
                        Constant declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        Variable declarations
------------------------------------------------------------------------------*/
char debugSerialNumber[9] = {0};
/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/


esp_err_t httpReqSerailNumberGet(httpd_req_t *req)
{
  /**
   * get current serial Number
   */
  uint32_t serialNumber = 0;
  // TODO get stored number
  cJSON* state = NULL;
  if (serialNumber == 0)
  {
    state = cJSON_CreateString("serial-required");
  }
  else
  {
    state = cJSON_CreateString("serial-already-specified");
  }

  cJSON *monitor = cJSON_CreateObject();
  cJSON_AddItemToObject(monitor, "state", state);

  char* json = cJSON_Print(monitor);
  uint16_t len = strlen(json);
  httpd_resp_set_type(req, "application/json");  // Set content type to text/plain
  httpd_resp_send(req, json, len);

  cJSON_free(json);
  cJSON_Delete(monitor);

  return ESP_OK;
}

esp_err_t httpReqSerailNumberPut(httpd_req_t *req)
{

  char buf;
  int ret;

  char rxBuffer[200];
  uint16_t len = sizeof(rxBuffer);

  if  (len > req->content_len)
  {
    len = req->content_len;
  }

  ret = httpd_req_recv(req, rxBuffer, len);

  if (ret <= 0)
  {
    httpd_resp_send_408(req);
    return ESP_FAIL;
  }
  else
  {
    cJSON* content = cJSON_Parse(rxBuffer); // Parse JSON data

    if (content == NULL) {
        // Parsing failed
        printf("Failed to parse JSON data.\n");
        return ESP_FAIL;
    }

    // Extract values from JSON object
    cJSON* serialNumber = cJSON_GetObjectItem(content, "serialNumber");
    cJSON* url = cJSON_GetObjectItem(content, "url");

    if (serialNumber != NULL && serialNumber->type == cJSON_String)
    {
      strcpy(debugSerialNumber, serialNumber->valuestring);
      printf("serial number: %s\n", debugSerialNumber);
    }
    if (url != NULL && url->type == cJSON_String)
    {
        printf("url: %s\n", url->valuestring);
    }

    cJSON_Delete(content);
  }

  /* Respond with empty body */
  httpd_resp_send(req, NULL, 0);

  return ESP_OK;
}


esp_err_t httpReqSsidAllGet(httpd_req_t *req)
{
  /**
   * get all SSID's available.
   */
  wifi_ap_record_t record[20];
  uint16_t count;
  count = APP::wifiap_scan(record, 20);

  // Create a JSON array for ssids
  cJSON* ssidsArray = cJSON_CreateArray();

  printf("SSID count: %d\n", count);

  int n;
  for(n = 0; n < count; n++)
  {
    // Create individual ssid objects and add them to the ssids array
    cJSON* ssid = cJSON_CreateObject();
    char const* str = reinterpret_cast<const char*>(record[n].ssid);
    printf("SSID %d: %s\n", n, str);
    cJSON_AddItemToObject(ssid, "name", cJSON_CreateString(str));
    cJSON_AddItemToObject(ssid, "signalStrength", cJSON_CreateNumber(record[n].rssi));
    cJSON_AddItemToArray(ssidsArray, ssid);
  }

  // Create the root JSON object and add the ssids array to it
  cJSON* root = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "ssids", ssidsArray);

  // Print the JSON object as a string
  char* jsonString = cJSON_Print(root);

  printf("\n%s\n", jsonString);

  uint16_t len = strlen(jsonString);
  httpd_resp_set_type(req, "application/json");  // Set content type to text/plain
  httpd_resp_send(req, jsonString, len);

  // Don't forget to free the allocated memory for the JSON string and objects
  cJSON_free(jsonString);
  cJSON_Delete(root);

  return ESP_OK;
}


esp_err_t httpReqSsidAllPut(httpd_req_t *req)
{

  char buf;
  int ret;

  char rxBuffer[200];
  uint16_t len = sizeof(rxBuffer);

  if  (len > req->content_len)
  {
    len = req->content_len;
  }

  ret = httpd_req_recv(req, rxBuffer, len);

  if (ret <= 0)
  {
    httpd_resp_send_408(req);
    return ESP_FAIL;
  }
  else
  {
    cJSON* content = cJSON_Parse(rxBuffer); // Parse JSON data

    if (content == NULL) {
        // Parsing failed
        printf("Failed to parse JSON data.\n");
        return ESP_FAIL;
    }

    // Extract values from JSON object
    cJSON* ssid = cJSON_GetObjectItem(content, "ssid");
    cJSON* password = cJSON_GetObjectItem(content, "password");

    if (ssid != NULL && ssid->type == cJSON_String)
    {
        printf("ssid: %s\n", ssid->valuestring);
    }
    if (password != NULL && password->type == cJSON_String)
    {
        printf("password: %s\n", password->valuestring);
    }

    cJSON_Delete(content);
  }

  /* Respond with empty body */
  httpd_resp_send(req, NULL, 0);

  return ESP_OK;
}

esp_err_t httpReqSettingsGet(httpd_req_t *req)
{
  /**
   * get all settings.
   */
  // Create cJSON objects for nested structures
  cJSON* ampsAllowedArray = cJSON_CreateArray();
  cJSON_AddItemToArray(ampsAllowedArray, cJSON_CreateNumber(10));
  cJSON_AddItemToArray(ampsAllowedArray, cJSON_CreateNumber(12));
  cJSON_AddItemToArray(ampsAllowedArray, cJSON_CreateNumber(16));

  cJSON* ampsObject = cJSON_CreateObject();
  cJSON_AddItemToObject(ampsObject, "allowed", ampsAllowedArray);
  cJSON_AddItemToObject(ampsObject, "current", cJSON_CreateNumber(16));

  cJSON* ocppObject = cJSON_CreateObject();
  cJSON_AddItemToObject(ocppObject, "url", cJSON_CreateString(""));

  // Create the root cJSON object and add nested objects and values
  cJSON* jsonRoot = cJSON_CreateObject();
  cJSON_AddItemToObject(jsonRoot, "amps", ampsObject);
  cJSON_AddItemToObject(jsonRoot, "firmwareVersion", cJSON_CreateString("0.0.1"));
  cJSON_AddItemToObject(jsonRoot, "ocpp", ocppObject);
  cJSON_AddItemToObject(jsonRoot, "serialNumber", cJSON_CreateString(debugSerialNumber));

  // Convert cJSON object to JSON string
  char* jsonString = cJSON_Print(jsonRoot);
  if (jsonString == nullptr)
  {
    printf("Failed to createt JSON data.\n");
    cJSON_Delete(jsonRoot);

    return ESP_FAIL;
  }

  // Print the JSON string
  printf("\n%s\n", jsonString);

  uint16_t len = strlen(jsonString);
  httpd_resp_set_type(req, "application/json");  // Set content type to text/plain
  httpd_resp_send(req, jsonString, len);

  // Free allocated memory
  cJSON_free(jsonString);
  cJSON_Delete(jsonRoot);

  return ESP_OK;
}

esp_err_t httpReqSettingsPut (httpd_req_t *req)
{

  char buf;
  int ret;

  char rxBuffer[200];
  uint16_t len = sizeof(rxBuffer);

  if (len > req->content_len)
  {
    len = req->content_len;
  }

  ret = httpd_req_recv(req, rxBuffer, len);

  if (ret <= 0)
  {
    httpd_resp_send_408(req);
    return ESP_FAIL;
  }
  else
  {
    cJSON *content = cJSON_Parse(rxBuffer); // Parse JSON data

    if (content == NULL)
    {
      // Parsing failed
      printf("Failed to parse JSON data.\n");
      return ESP_FAIL;
    }

    printf("Parse PUT settings\n");
    cJSON *ampsObject = cJSON_GetObjectItem(content, "amps");
    cJSON *ocppObject = cJSON_GetObjectItem(content, "ocpp");

    if (ocppObject != nullptr)
    {
      cJSON* url = cJSON_GetObjectItem(ocppObject, "url");
      if (url != NULL && url->type == cJSON_String)
      {
        printf("occp url: %s\n", url->valuestring);
      }
    }

    if (ampsObject != nullptr)
    {
      cJSON *current = cJSON_GetObjectItem(ampsObject, "current");
      if (current != NULL && current->type == cJSON_Number)
      {
        printf("current: %d\n", current->valueint);
      }
    }

    cJSON_Delete(content);
  }

  /* Respond with empty body */
  httpd_resp_send(req, NULL, 0);

  return ESP_OK;
}

}  // anonymous namespace

/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace APP
{

void taskApp( void *arg )
{




  auto webServer = Webserver();

  webServer.getCbAdd("/iq-plug/discover", httpReqSerailNumberGet);
  webServer.putCbAdd("/iq-plug/discover", httpReqSerailNumberPut);

  webServer.getCbAdd("/api/network/ssid/all", httpReqSsidAllGet);
  webServer.putCbAdd("/api/network/ssid", httpReqSsidAllPut);

  webServer.getCbAdd("/api/settings", httpReqSettingsGet);
  webServer.putCbAdd("/api/settings", httpReqSettingsPut);

  printf("webServer URI GET CB created.");

  while(1)
  {

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    printf("APP, free heap size: %d\n", esp_get_free_heap_size());

  }

}


}  // namespace end APP
