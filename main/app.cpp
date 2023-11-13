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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "app.h"
#include "webserver.h"
#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "cJSON.h"

#include "udpDaemon.h"
#include "wifiap.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/

namespace // anonymous
{
  /*------------------------------------------------------------------------------
                          Constant declarations
  ------------------------------------------------------------------------------*/

  const int udpPort = 8370;
  const char udpReqString[] = "IQ-PLUG_CHARGER_DISCOVERY";
  const char udpResString[] = "IQ-PLUG_CHARGER_FOUND_"; // + serial number

  const char fileDeployment[] = "/app/deploy_data";
  const char fileWifi[] = "/app/wifi_settings";
  const char fileOcpp[] = "/app/ocpp_settings";
  /*------------------------------------------------------------------------------
                          Type declarations
  ------------------------------------------------------------------------------*/

  typedef enum
  {
    DATA_UPDATE_EVENT_SERIAL_NO = 0x01,
    DATA_UPDATE_EVENT_URL = 0x02,
    DATA_UPDATE_EVENT_SSID = 0x04,
    DATA_UPDATE_EVENT_PASSWORD = 0x08,
    DATA_UPDATE_EVENT_AMPS = 0x10,
    DATA_UPDATE_EVENT_OCPP = 0x20,
    DATA_UPDATE_EVENT_PHASES_AVAILABLE = 0x40,
    DATA_UPDATE_EVENT_PHASE_ORDER = 0x80,
    DATA_UPDATE_EVENT_ALL = 0xFF,
  } dataUpdateEventGroup_t;

  typedef struct
  {
    uint32_t version;
    int serialNumber;
    char url[35];
    int currentAllowed;
    int phaseAvailable;
    char phaseOrder[10];
  } deployment_t;

  typedef struct
  {
    uint32_t version;
    char url[150];
  } ocpp_t;

  typedef struct
  {
    uint32_t version;
    char Ssid[33];
    char password[64];
  } wifi_t;

  /*------------------------------------------------------------------------------
                          Variable declarations
  ------------------------------------------------------------------------------*/
  static EventGroupHandle_t dataUpdateEventGroup = NULL;

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
  void setReqHeaders(httpd_req_t *req)
  {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
  }

  esp_err_t preflighRequestHandler(httpd_req_t *req) {
    setReqHeaders(req); // Set the CORS headers
    httpd_resp_send(req, NULL, 0); // No need to send any response body for OPTIONS
    return ESP_OK;
  }

  esp_err_t httpReqSerailNumberGet(httpd_req_t *req)
  {
    /**
     * get current serial Number
     */
    // TODO get stored number
    setReqHeaders(req);
    cJSON *state = NULL;
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

    char *json = cJSON_Print(monitor);
    uint16_t len = strlen(json);
    httpd_resp_set_type(req, "application/json"); // Set content type to text/plain
    httpd_resp_send(req, json, len);

    cJSON_free(json);
    cJSON_Delete(monitor);

    return ESP_OK;
  }

  esp_err_t httpReqSerailNumberPut(httpd_req_t *req)
  {
    setReqHeaders(req);
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
          xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_SERIAL_NO);
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
          xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_URL);
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
    setReqHeaders(req);
    wifi_ap_record_t record[20];
    uint16_t count;
    count = APP::wifiap_scan(record, 20);

    // Create a JSON array for ssids
    cJSON *ssidsArray = cJSON_CreateArray();

    printf("SSID count: %d\n", count);

    int n;
    for (n = 0; n < count; n++)
    {
      // Create individual ssid objects and add them to the ssids array
      cJSON *ssid = cJSON_CreateObject();
      char const *str = reinterpret_cast<const char *>(record[n].ssid);
      printf("SSID %d: %s\n", n, str);
      cJSON_AddItemToObject(ssid, "name", cJSON_CreateString(str));
      cJSON_AddItemToObject(ssid, "signalStrength", cJSON_CreateNumber(record[n].rssi));
      cJSON_AddItemToArray(ssidsArray, ssid);
    }

    // Create the root JSON object and add the ssids array to it
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "ssids", ssidsArray);

    // Print the JSON object as a string
    char *jsonString = cJSON_Print(root);

    printf("\n%s\n", jsonString);

    uint16_t len = strlen(jsonString);
    httpd_resp_set_type(req, "application/json"); // Set content type to text/plain
    httpd_resp_send(req, jsonString, len);

    // Don't forget to free the allocated memory for the JSON string and objects
    cJSON_free(jsonString);
    cJSON_Delete(root);

    return ESP_OK;
  }

  esp_err_t httpReqSsidAllPut(httpd_req_t *req)
  {
    setReqHeaders(req);
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

      // Extract values from JSON object
      cJSON *ssid = cJSON_GetObjectItem(content, "ssid");
      cJSON *password = cJSON_GetObjectItem(content, "password");

      if (ssid != NULL && ssid->type == cJSON_String)
      {
        strcpy(debugSsid, ssid->valuestring);
        printf("ssid: %s\n", ssid->valuestring);
        xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_SSID);
      }
      if (password != NULL && password->type == cJSON_String)
      {
        strcpy(debugWifiPaasword, password->valuestring);
        printf("password: %s\n", debugWifiPaasword);
        xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_PASSWORD);
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
    setReqHeaders(req);
    // Create cJSON objects for nested structures
    cJSON *ampsAllowedArray = cJSON_CreateArray();
    cJSON_AddItemToArray(ampsAllowedArray, cJSON_CreateNumber(10));
    cJSON_AddItemToArray(ampsAllowedArray, cJSON_CreateNumber(12));
    cJSON_AddItemToArray(ampsAllowedArray, cJSON_CreateNumber(16));

    cJSON *ampsObject = cJSON_CreateObject();
    cJSON_AddItemToObject(ampsObject, "allowed", ampsAllowedArray);
    cJSON_AddItemToObject(ampsObject, "current", cJSON_CreateNumber(debugCurrentAllowed));

    cJSON *ocppObject = cJSON_CreateObject();
    cJSON_AddItemToObject(ocppObject, "url", cJSON_CreateString(debugOcppUrl));

    cJSON *phaseAvailableArray = cJSON_CreateArray();
    cJSON_AddItemToArray(phaseAvailableArray, cJSON_CreateNumber(1));
    cJSON_AddItemToArray(phaseAvailableArray, cJSON_CreateNumber(3));

    cJSON *phaseAvailableObject = cJSON_CreateObject();
    cJSON_AddItemToObject(phaseAvailableObject, "allowed", phaseAvailableArray);
    cJSON_AddItemToObject(phaseAvailableObject, "current", cJSON_CreateNumber(debugPhaseAvailable));

    cJSON *phasesOrderArray = cJSON_CreateArray();
    cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L1-L2-L3"));
    cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L1-L3-L2"));
    cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L2-L1-L3"));
    cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L2-L3-L1"));
    cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L3-L1-L2"));
    cJSON_AddItemToArray(phasesOrderArray, cJSON_CreateString("L3-L2-L1"));

    cJSON *phaseOrderObject = cJSON_CreateObject();
    cJSON_AddItemToObject(phaseOrderObject, "allowed", phasesOrderArray);
    cJSON_AddItemToObject(phaseOrderObject, "current", cJSON_CreateString(debugPhaseOrder));

    // Create the root cJSON object and add nested objects and values
    cJSON *jsonRoot = cJSON_CreateObject();
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
    char *jsonString = cJSON_Print(jsonRoot);
    if (jsonString == nullptr)
    {
      printf("Failed to createt JSON data.\n");
      cJSON_Delete(jsonRoot);

      return ESP_FAIL;
    }

    // Print the JSON string
    // printf("\n%s\n", jsonString);

    uint16_t len = strlen(jsonString);
    httpd_resp_set_type(req, "application/json"); // Set content type to text/plain
    httpd_resp_send(req, jsonString, len);

    // Free allocated memory
    cJSON_free(jsonString);
    cJSON_Delete(jsonRoot);

    return ESP_OK;
  }

  esp_err_t httpReqSettingsPut(httpd_req_t *req)
  {
    setReqHeaders(req);
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
        cJSON *url = cJSON_GetObjectItem(ocppObject, "url");
        if (url != NULL && url->type == cJSON_String)
        {
          strcpy(debugOcppUrl, url->valuestring);
          printf("occp url: %s\n", url->valuestring);
          xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_OCPP);
        }
      }

      if (ampsObject != nullptr)
      {
        cJSON *current = cJSON_GetObjectItem(ampsObject, "current");
        if (current != NULL && current->type == cJSON_Number)
        {
          debugCurrentAllowed = current->valueint;
          printf("current: %d\n", current->valueint);
          xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_AMPS);
        }
      }

      if (phaseOrderObject != nullptr)
      {
        cJSON *current = cJSON_GetObjectItem(phaseOrderObject, "current");
        if (current != NULL && current->type == cJSON_String)
        {
          strcpy(debugPhaseOrder, current->valuestring);
          printf("phases order: %s\n", current->valuestring);
          xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_PHASE_ORDER);
        }
      }

      if (phaseAvailableObject != nullptr)
      {
        cJSON *current = cJSON_GetObjectItem(phaseAvailableObject, "current");
        if (current != NULL && current->type == cJSON_Number)
        {
          debugPhaseAvailable = current->valueint;
          printf("phases available: %d\n", current->valueint);
          xEventGroupSetBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_PHASES_AVAILABLE);
        }
      }

      cJSON_Delete(content);
    }

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
  }

} // anonymous namespace

/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace APP
{

  void wifiDataStore(void)
  {
    // wifi
    wifi_t wifiData;
    memset(&wifiData, 0, sizeof(wifiData));
    FILE *fh = fopen(fileWifi, "w");

    if (fh == nullptr)
    {
      ESP_LOGI(__FILE__, "Can not open file %s", fileWifi);
    }
    else
    {
      // set data to write
      wifiData.version = 1;
      strcpy(wifiData.Ssid, debugSsid);
      strcpy(wifiData.password, debugWifiPaasword);

      int count;
      int fileSize = sizeof(wifiData);
      count = fwrite(&wifiData, fileSize, 1, fh);
      fclose(fh);

      if (count != 1)
      {
        ESP_LOGE(__FILE__, "File not written correctly: %s: %d expected: %d", fileWifi, count, fileSize);
      }
      else
      {
        ESP_LOGI(__FILE__, "%s updated.", fileWifi);
      }
    }
  }

  void wifiDataRestore(void)
  {
    // wifi
    wifi_t wifiData;
    memset(&wifiData, 0, sizeof(wifiData));
    FILE *fh = fopen(fileWifi, "r");

    if (fh == nullptr)
    {
      ESP_LOGI(__FILE__, "Can not open file %s", fileWifi);
    }
    else
    {
      int count;
      int fileSize = sizeof(wifiData);
      count = fread(&wifiData, fileSize, 1, fh);
      fclose(fh);

      if (count != 1)
      {
        ESP_LOGE(__FILE__, "File not read correctly: %s: %d", fileWifi, count);
      }
      else
      {
        if (wifiData.version != 1)
        {
          ESP_LOGE(__FILE__, "Do not support this version of the data structure: %d", wifiData.version);
        }
        else
        {
          // restore data
          strcpy(debugSsid, wifiData.Ssid);
          strcpy(debugWifiPaasword, wifiData.password);
          ESP_LOGI(__FILE__, "%s restored.", fileWifi);
        }
      }
    }
  }

  void deploymentDataStore(void)
  {
    // deployment
    deployment_t deploymentData;
    memset(&deploymentData, 0, sizeof(deploymentData));
    FILE *fh = fopen(fileDeployment, "w");

    if (fh == nullptr)
    {
      ESP_LOGI(__FILE__, "Can not open file %s", fileDeployment);
    }
    else
    {
      // set data to write
      deploymentData.version = 1;
      deploymentData.serialNumber = debugSerialNumber;
      strcpy(deploymentData.url, debugIqUrl);
      deploymentData.currentAllowed = debugCurrentAllowed;
      deploymentData.phaseAvailable = debugPhaseAvailable;
      strcpy(deploymentData.phaseOrder, debugPhaseOrder);

      int count;
      int fileSize = sizeof(deploymentData);
      count = fwrite(&deploymentData, fileSize, 1, fh);
      fclose(fh);

      if (count != 1)
      {
        ESP_LOGE(__FILE__, "File not written correctly: %s: %d expected: %d", fileDeployment, count, fileSize);
      }
      else
      {
        ESP_LOGI(__FILE__, "%s updated.", fileDeployment);
      }
    }
  }

  void deploymentDataRestore(void)
  {
    // deployment
    deployment_t deploymentData;
    memset(&deploymentData, 0, sizeof(deploymentData));
    FILE *fh = fopen(fileDeployment, "r");

    if (fh == nullptr)
    {
      ESP_LOGI(__FILE__, "Can not open file %s", fileDeployment);
    }
    else
    {
      int count;
      int fileSize = sizeof(deploymentData);
      count = fread(&deploymentData, fileSize, 1, fh);
      fclose(fh);

      if (count != 1)
      {
        ESP_LOGE(__FILE__, "File not read correctly: %s: %d", fileDeployment, count);
      }
      else
      {
        if (deploymentData.version != 1)
        {
          ESP_LOGE(__FILE__, "Do not support this version of the data structure: %d", deploymentData.version);
        }
        else
        {
          // restore data

          debugSerialNumber = deploymentData.serialNumber;
          strcpy(debugIqUrl, deploymentData.url);
          debugCurrentAllowed = deploymentData.currentAllowed;
          debugPhaseAvailable = deploymentData.phaseAvailable;
          strcpy(debugPhaseOrder, deploymentData.phaseOrder);

          ESP_LOGI(__FILE__, "%s restored.", fileDeployment);
        }
      }
    }
  }

  void ocppDataStore(void)
  {
    // ocpp
    ocpp_t ocppData;
    memset(&ocppData, 0, sizeof(ocppData));

    FILE *fh = fopen(fileOcpp, "w");

    if (fh == nullptr)
    {
      ESP_LOGI(__FILE__, "Can not open file %s", fileOcpp);
    }
    else
    {
      // set data to write
      ocppData.version = 1;
      strcpy(ocppData.url, debugOcppUrl);

      int count;
      int fileSize = sizeof(ocppData);
      count = fwrite(&ocppData, fileSize, 1, fh);
      fclose(fh);

      if (count != 1)
      {
        ESP_LOGE(__FILE__, "File not written correctly: %s: %d expected: %d", fileOcpp, count, fileSize);
      }
      else
      {
        ESP_LOGI(__FILE__, "%s updated.", fileOcpp);
      }
    }
  }

  void ocppDataRestore(void)
  {
    // ocpp
    ocpp_t ocppData;
    memset(&ocppData, 0, sizeof(ocppData));

    FILE *fh = fopen(fileOcpp, "r");

    if (fh == nullptr)
    {
      ESP_LOGI(__FILE__, "Can not open file %s", fileOcpp);
    }
    else
    {
      int count;
      int fileSize = sizeof(ocppData);
      count = fread(&ocppData, fileSize, 1, fh);
      fclose(fh);

      if (count != 1)
      {
        ESP_LOGE(__FILE__, "File not read correctly: %s: %d", fileOcpp, count);
      }
      else
      {
        if (ocppData.version != 1)
        {
          ESP_LOGE(__FILE__, "Do not support this version of the data structure: %d", ocppData.version);
        }
        else
        {
          // restore data
          strcpy(debugOcppUrl, ocppData.url);

          ESP_LOGI(__FILE__, "%s restored.", fileOcpp);
        }
      }
    }
  }

  void taskApp(void *arg)
  {
    esp_vfs_spiffs_conf_t conf =
        {
            .base_path = "/app",
            .partition_label = NULL,
            .max_files = 3,
            .format_if_mount_failed = true};

    // Mount SPIFFS
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
      ESP_LOGE(__FILE__, "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
      while (1)
        ;
    }

    // SPIFFS mounted successfully
    ESP_LOGI(__FILE__, "SPIFFS mounted successfully");

    wifiDataRestore();
    deploymentDataRestore();
    ocppDataRestore();

    wifiap_init(); // init wifi and event handler
    if (strlen(debugSsid) > 0)
    {
      ESP_ERROR_CHECK(wifiap_connectToAp(debugSsid, debugWifiPaasword));
    }
    else
    {
      ESP_ERROR_CHECK(wifiap_startAp());
    }

    dataUpdateEventGroup = xEventGroupCreate();

    auto webServer = Webserver();

    webServer.getCbAdd("/iq-plug/discover", httpReqSerailNumberGet);
    webServer.putCbAdd("/iq-plug/discover", httpReqSerailNumberPut);

    webServer.optionsCbAdd("/iq-plug/discover", preflighRequestHandler);

    webServer.getCbAdd("/api/network/ssid/all", httpReqSsidAllGet);
    webServer.putCbAdd("/api/network/ssid", httpReqSsidAllPut);

    webServer.optionsCbAdd("/api/network/ssid/all", preflighRequestHandler);
    webServer.optionsCbAdd("/api/network/ssid", preflighRequestHandler);

    webServer.getCbAdd("/api/settings", httpReqSettingsGet);
    webServer.putCbAdd("/api/settings", httpReqSettingsPut);

    webServer.optionsCbAdd("/api/settings", preflighRequestHandler);

    printf("webServer URI GET CB created.");

    uint32_t delayMs = 1000;

    while (1)
    {
      /**
       * set host name
       */
      char const *currentHostname;
      if (tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &currentHostname) == ESP_OK)
      {
        char hostname[20];
        snprintf(hostname, sizeof(hostname), "IQ-home_%08d", debugSerialNumber);

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
        else
        {
          /**
           * run UDP socket if wifi connection is up
           */
          if (wifiap_isWifiUp() == true)
          {
            char txData[40];
            snprintf((char *)txData, sizeof(txData), "%s%08d\n", udpResString, debugSerialNumber);
            UdpDaemon_broadcastStart(udpPort, udpReqString, txData);
          }
        }
      }

      /**
       * Write data to file
       */
      uint32_t flags;
      flags = xEventGroupWaitBits(dataUpdateEventGroup, DATA_UPDATE_EVENT_ALL, true, false, 0);

      if (flags > 0)
      {
        ESP_LOGI(__FILE__, "SAVE data to file, flags: %02X", flags);

        if ((flags & DATA_UPDATE_EVENT_SSID) ||
            (flags & DATA_UPDATE_EVENT_PASSWORD))
        {
          // wifi
          wifiDataStore();
        }
        if ((flags & DATA_UPDATE_EVENT_AMPS) ||
            (flags & DATA_UPDATE_EVENT_PHASE_ORDER) ||
            (flags & DATA_UPDATE_EVENT_SERIAL_NO) ||
            (flags & DATA_UPDATE_EVENT_URL) ||
            (flags & DATA_UPDATE_EVENT_PHASES_AVAILABLE))
        {
          // deployment
          deploymentDataStore();
        }
        if (flags & DATA_UPDATE_EVENT_OCPP)
        {
          // deployment
          ocppDataStore();
        }
      }

      printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
      vTaskDelay(delayMs / portTICK_PERIOD_MS);
    }
  }

} // namespace end APP
