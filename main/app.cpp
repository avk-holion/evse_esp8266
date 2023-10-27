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
#include <string>

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
char debugFirmwareVersion[] = "0.0.0.1";
int debugSerialNumber = 0;
char debugIqUrl[35] = "";
int debugCurrentAllowed = 16;
int debugPhaseAvailable = 3;
char debugPhaseOrder[] = "L1-L2-L3";
char debugOcppUrl[150] = "";
char debugSsid[33] = "";
char debugWifiPaasword[64];

/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/


esp_err_t httpReqSerailNumberGet(httpd_req_t *req)
{
  /**
   * get current serial Number
   */
  // TODO get stored number
  cJSON* state = NULL;
  if (debugSerialNumber == 0)
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

esp_err_t httpReqSerailNumberPut (httpd_req_t *req)
{

  char buf;
  int ret;
  esp_err_t status = ESP_OK;

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
    status = ESP_FAIL;
  }
  else
  {
    cJSON *content = cJSON_Parse(rxBuffer); // Parse JSON data

    if (content == NULL)
    {
      // Parsing failed
      printf("Failed to parse JSON data.\n");
      status = ESP_FAIL;
    }
    else
    {

      // Extract values from JSON object
      cJSON *serialNumber = cJSON_GetObjectItem(content, "serialNumber");
      cJSON *url = cJSON_GetObjectItem(content, "url");

      if (serialNumber == NULL)
      {
        printf("Failed to parse JSON \"serialNumber\" data.\n");
        status = ESP_FAIL;
      }
      else if (serialNumber->type != cJSON_String)
      {
        printf("Failed to parse JSON serial number type.\n");
        status = ESP_FAIL;
      }
      else if (sscanf(serialNumber->valuestring, "%d", &debugSerialNumber) != 1)
      {
        printf("Failed to parse JSON serial number data.\n");
        status = ESP_FAIL;
      }
      else
      {
        printf("serial number: %d\n", debugSerialNumber);
      }

      if (url == NULL)
      {
        printf("Failed to parse JSON \"serial number url\" data.\n");
        status = ESP_FAIL;
      }
      else if (url->type != cJSON_String)
      {
        printf("Failed to parse JSON serial number url type.\n");
        status = ESP_FAIL;
      }
      else
      {
        strcpy(debugIqUrl, url->valuestring);
        printf("url: %s\n", url->valuestring);
      }

      cJSON_Delete(content);
    }
  }

  /* Respond with empty body */
  httpd_resp_send(req, NULL, 0);

  return status;
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
      strcpy(debugSsid, ssid->valuestring);
        printf("ssid: %s\n", ssid->valuestring);
    }
    if (password != NULL && password->type == cJSON_String)
    {
      strcpy(debugWifiPaasword, password->valuestring);
        printf("password: %s\n", debugWifiPaasword);
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
  cJSON_AddItemToObject(ampsObject, "current", cJSON_CreateNumber(debugCurrentAllowed));

  cJSON* ocppObject = cJSON_CreateObject();
  cJSON_AddItemToObject(ocppObject, "url", cJSON_CreateString(debugOcppUrl));

  cJSON* phaseAvailableArray = cJSON_CreateArray();
  cJSON_AddItemToArray(phaseAvailableArray, cJSON_CreateNumber(1));
  cJSON_AddItemToArray(phaseAvailableArray, cJSON_CreateNumber(3));

  cJSON* phaseAvailableObject = cJSON_CreateObject();
  cJSON_AddItemToObject(phaseAvailableObject, "allowed", phaseAvailableArray);
  cJSON_AddItemToObject(phaseAvailableObject, "current", cJSON_CreateNumber(debugPhaseAvailable));

  cJSON* phasesOrderArray = cJSON_CreateArray();
  cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L1-L2-L3"));
  cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L1-L3-L2"));
  cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L2-L1-L3"));
  cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L2-L3-L1"));
  cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L3-L1-L2"));
  cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L3-L2-L1"));

  cJSON* phaseOrderObject = cJSON_CreateObject();
  cJSON_AddItemToObject(phaseOrderObject, "allowed", phasesOrderArray);
  cJSON_AddItemToObject(phaseOrderObject, "current", cJSON_CreateString(debugPhaseOrder));

  // Create the root cJSON object and add nested objects and values
  cJSON* jsonRoot = cJSON_CreateObject();
  cJSON_AddItemToObject(jsonRoot, "amps", ampsObject);
  cJSON_AddItemToObject(jsonRoot, "firmwareVersion", cJSON_CreateString(debugFirmwareVersion));
  cJSON_AddItemToObject(jsonRoot, "ocpp", ocppObject);
  cJSON_AddItemToObject(jsonRoot, "phaseAvailable", phaseAvailableObject);
  cJSON_AddItemToObject(jsonRoot, "phaseOrder", phaseOrderObject);
  char strSerialNumber[12] = {0};
  printf("serial number: %d\n", debugSerialNumber);
  snprintf(strSerialNumber, sizeof(strSerialNumber), "%d", debugSerialNumber);
  cJSON_AddItemToObject(jsonRoot, "serialNumber", cJSON_CreateString(strSerialNumber));

  // Convert cJSON object to JSON string
  char* jsonString = cJSON_Print(jsonRoot);
  if (jsonString == nullptr)
  {
    printf("Failed to createt JSON data.\n");
    cJSON_Delete(jsonRoot);

    return ESP_FAIL;
  }

  // Print the JSON string
  // printf("\n%s\n", jsonString);

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
    cJSON *phaseAvailableObject = cJSON_GetObjectItem(content, "phaseAvailable");
    cJSON *phaseOrderObject = cJSON_GetObjectItem(content, "phaseOrder");

    if (ocppObject != nullptr)
    {
      cJSON* url = cJSON_GetObjectItem(ocppObject, "url");
      if (url != NULL && url->type == cJSON_String)
      {
        strcpy(debugOcppUrl, url->valuestring);
        printf("occp url: %s\n", url->valuestring);
      }
    }

    if (ampsObject != nullptr)
    {
      cJSON *current = cJSON_GetObjectItem(ampsObject, "current");
      if (current != NULL && current->type == cJSON_Number)
      {
        debugCurrentAllowed = current->valueint;
        printf("current: %d\n", current->valueint);
      }
    }

    if (phaseOrderObject != nullptr)
    {
      cJSON* current = cJSON_GetObjectItem(phaseOrderObject, "current");
      if (current != NULL && current->type == cJSON_String)
      {
        strcpy(debugPhaseOrder, current->valuestring);
        printf("phases order: %s\n", current->valuestring);
      }
    }

    if (phaseAvailableObject != nullptr)
    {
      cJSON *current = cJSON_GetObjectItem(phaseAvailableObject, "current");
      if (current != NULL && current->type == cJSON_Number)
      {
        debugPhaseAvailable = current->valueint;
        printf("phases available: %d\n", current->valueint);
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
    /**
     * set host name
     */
    char const* currentHostname;
    if (tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &currentHostname) == ESP_OK)
    {
      char hostname[20];
      snprintf(hostname, sizeof(hostname),"IQ-home_%08d", debugSerialNumber );

      printf("host name: %s\n", currentHostname);
      if (strcmp(currentHostname, hostname) != 0)
      {
        printf("new host name: %s\n", hostname);
        ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hostname));
      }
    }

    /**
     * check for connected to AP/wifi
     */
    if (strlen(debugSsid) > 0)
    {
      if (wifiap_isApMode() == true)
      {
        ESP_ERROR_CHECK(wifiap_connectToAp(debugSsid, debugWifiPaasword));
      }
    }

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    printf("APP, free heap size: %d\n", esp_get_free_heap_size());

  }

}


}  // namespace end APP
