/*------------------------------------------------------------------------------
* debug.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 9. okt. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "debug.h"
#include "webserver.h"
#include "evse.h"
#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "cJSON.h"
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
static APP::Webserver* myWebServer = nullptr;
/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/
static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    ESP_LOGI(__FILE__, "connect_handler");
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(__FILE__, "Starting webserver");

        static auto ws = APP::Webserver();
        myWebServer = &ws;
        *server = myWebServer->server;
    }
}

static char* jsonTest(void)
{
  cJSON* state = NULL;
  state = cJSON_CreateString("serial-required");

  cJSON *monitor = cJSON_CreateObject();
  cJSON_AddItemToObject(monitor, "state", state);

  return cJSON_Print(monitor);
}

}  // anonymous namespace

/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace APP
{

esp_err_t httpReqInfo(httpd_req_t *req)
{
  /* Send a response */
  std::string output = "IQ-home info page.";
  output.append("<BR>");
  output.append("Information on this page are for information only.");

  output.append("<BR>");
  output.append("STA IP: address:");
  tcpip_adapter_ip_info_t tcpipSta;
  tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &tcpipSta);
  char ip4[17];
  sprintf(ip4, IPSTR, IP2STR(&tcpipSta.ip));
  output.append(ip4);

  output.append("<BR>");
  output.append("AP IP: address:");

  tcpip_adapter_ip_info_t tcpipAp;
  tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &tcpipAp);
  sprintf(ip4, IPSTR, IP2STR(&tcpipAp.ip));
  output.append(ip4);

//  httpd_resp_send(req, output.data(), output.length());

  std::string html = "\
  <html>\n\
  <head>\n\
    <title>IQ-home live data</title>\n\
    <style>\n\
      table {\n\
        border-collapse: collapse;\n\
        width: 100%;\n\
      }\n\
      th, td {\n\
        border: 1px solid black;\n\
        padding: 8px;\n\
        text-align: left;\n\
      }\n\
      th {\n\
        background-color: #f2f2f2;\n\
      }\n\
    </style>\n\
  </head>\n\
  <body>\n\
\n\
  <h2>IQ-home live data</h2>\n\
<table id=\"data-table\">\n\
  <tr>\n\
    <th>Name</th>\n\
    <th>Value</th>\n\
    <!-- Add more table headers as needed -->\n\
  </tr>\n\
  </table>\n\
  <script>\n\
  function updateTable() {\n\
    var xhttp = new XMLHttpRequest();\n\
    xhttp.onreadystatechange = function() {\n\
      if (this.readyState == 4 && this.status == 200) {\n\
        var data = JSON.parse(this.responseText);\n\
        var table = document.getElementById(\"data-table\");\n\
        table.innerHTML = \"<tr><th>Name</th><th>Value</th></tr>\";\n\
        for (var i = 0; i < data.length; i++) {\n\
          var row = table.insertRow();\n\
          var cell1 = row.insertCell(0);\n\
          var cell2 = row.insertCell(1);\n\
          cell1.innerHTML = data[i].name;\n\
          cell2.innerHTML = data[i].value;\n\
        }\n\
      }\n\
    };\n\
    xhttp.open(\"GET\", \"/livedata\", true);\n\
    xhttp.send();\n\
  }\n\
  setInterval(updateTable, 1000);\n\
  </script>\n\
  </body>\n\
  </html>";
  httpd_resp_send(req, html.data(), html.length());

  return ESP_OK;
}

esp_err_t httpReqLivedata(httpd_req_t *req)
{
  // Create an array to hold JSON objects
  cJSON* root = cJSON_CreateArray();

  WALLBOX_API::ModbusWallbox* regHandle;
  regHandle = evseRegisterHandleGet();

  char name[50];
  char value[32];


  cJSON* upTime = cJSON_CreateObject();
  cJSON_AddStringToObject(upTime, "upTime", "upTime");
  snprintf(value, sizeof(value), "%d", xTaskGetTickCount() / (1000 / portTICK_PERIOD_MS));
  cJSON_AddStringToObject(upTime, "value", value);
  cJSON_AddItemToArray(root, upTime);

  if (regHandle != nullptr)
  {

    // static data plug
    cJSON* staticData = cJSON_CreateObject();
    cJSON_AddStringToObject(staticData, "name", "Static data");
    cJSON_AddItemToArray(root, staticData);

    cJSON* hwRevPlug = cJSON_CreateObject();
    cJSON_AddStringToObject(hwRevPlug, "name", "HW revision plug");
    snprintf(value, sizeof(value), "%d", regHandle->hwRevPlug.get());
    cJSON_AddStringToObject(hwRevPlug, "value", value);
    cJSON_AddItemToArray(root, hwRevPlug);

    cJSON* hwRevWallboxPower = cJSON_CreateObject();
    cJSON_AddStringToObject(hwRevWallboxPower, "name", "HW revision wallbox power");
    snprintf(value, sizeof(value), "%d", regHandle->hwRevWbPower.get());
    cJSON_AddStringToObject(hwRevWallboxPower, "value", value);
    cJSON_AddItemToArray(root, hwRevWallboxPower);

    cJSON* hwRevWallboxMcu = cJSON_CreateObject();
    cJSON_AddStringToObject(hwRevWallboxMcu, "name", "HW revision wallbox mcu");
    snprintf(value, sizeof(value), "%d", regHandle->hwRevWbMcu.get());
    cJSON_AddStringToObject(hwRevWallboxMcu, "value", value);
    cJSON_AddItemToArray(root, hwRevWallboxMcu);

    cJSON* swRevPlug = cJSON_CreateObject();
    cJSON_AddStringToObject(swRevPlug, "name", "SW revision plug");
    snprintf(value, sizeof(value), "%d", regHandle->swRevPlug.get());
    cJSON_AddStringToObject(swRevPlug, "value", value);
    cJSON_AddItemToArray(root, swRevPlug);

    cJSON* swRevWallbox = cJSON_CreateObject();
    cJSON_AddStringToObject(swRevWallbox, "name", "SW revision wallbox");
    snprintf(value, sizeof(value), "%d", regHandle->swRevWallbox.get());
    cJSON_AddStringToObject(swRevWallbox, "value", value);
    cJSON_AddItemToArray(root, swRevWallbox);

    cJSON* swRevEsp = cJSON_CreateObject();
    cJSON_AddStringToObject(swRevEsp, "name", "SW revision esp");
    const esp_app_desc_t * otaDesc = esp_ota_get_app_description();

    snprintf(value, sizeof(value), "%s", otaDesc->version);
    cJSON_AddStringToObject(swRevEsp, "value", value);
    cJSON_AddItemToArray(root, swRevEsp);

    cJSON* statusSelftestPlug = cJSON_CreateObject();
    cJSON_AddStringToObject(statusSelftestPlug, "name", "Status selftest plug");
    snprintf(value, sizeof(value), "%d", regHandle->statusSelftestPlug.get());
    cJSON_AddStringToObject(statusSelftestPlug, "value", value);
    cJSON_AddItemToArray(root, statusSelftestPlug);

    cJSON* statusSelftestWallbox = cJSON_CreateObject();
    cJSON_AddStringToObject(statusSelftestWallbox, "name", "Status selftest wallbox");
    snprintf(value, sizeof(value), "%d", regHandle->statusSelftestWallbox.get());
    cJSON_AddStringToObject(statusSelftestWallbox, "value", value);
    cJSON_AddItemToArray(root, statusSelftestWallbox);

    cJSON* currentPhasesMaxPlug = cJSON_CreateObject();
    cJSON_AddStringToObject(currentPhasesMaxPlug, "name", "current max plug");
    snprintf(value, sizeof(value), "%d", regHandle->currentPhasesMaxPlug.get());
    cJSON_AddStringToObject(currentPhasesMaxPlug, "value", value);
    cJSON_AddItemToArray(root, currentPhasesMaxPlug);

    cJSON* currentPhasesMaxWallbox = cJSON_CreateObject();
    cJSON_AddStringToObject(currentPhasesMaxWallbox, "name", "current max wallbox");
    snprintf(value, sizeof(value), "%d", regHandle->currentPhasesMaxWallbox.get());
    cJSON_AddStringToObject(currentPhasesMaxWallbox, "value", value);
    cJSON_AddItemToArray(root, currentPhasesMaxWallbox);

    cJSON* deploymentData = cJSON_CreateObject();
    cJSON_AddStringToObject(deploymentData, "name", "Deployment data");
    cJSON_AddItemToArray(root, deploymentData);

    cJSON* serialNumber = cJSON_CreateObject();
    cJSON_AddStringToObject(serialNumber, "name", "serial number");
    snprintf(value, sizeof(value), "%08d", regHandle->serialNumber.get());
    cJSON_AddStringToObject(serialNumber, "value", value);
    cJSON_AddItemToArray(root, serialNumber);

    cJSON* currentMaxInstallation = cJSON_CreateObject();
    cJSON_AddStringToObject(currentMaxInstallation, "name", "Max installation current");
    snprintf(value, sizeof(value), "%d", regHandle->currentMaxInstallation.get());
    cJSON_AddStringToObject(currentMaxInstallation, "value", value);
    cJSON_AddItemToArray(root, currentMaxInstallation);

    cJSON* phasesOrderL1 = cJSON_CreateObject();
    cJSON_AddStringToObject(phasesOrderL1, "name", "phases order L1");
    snprintf(value, sizeof(value), "L%d", regHandle->phasesOrderL1.get());
    cJSON_AddStringToObject(phasesOrderL1, "value", value);
    cJSON_AddItemToArray(root, phasesOrderL1);

    cJSON* phasesOrderL2 = cJSON_CreateObject();
    cJSON_AddStringToObject(phasesOrderL2, "name", "phases order L2");
    snprintf(value, sizeof(value), "L%d", regHandle->phasesOrderL2.get());
    cJSON_AddStringToObject(phasesOrderL2, "value", value);
    cJSON_AddItemToArray(root, phasesOrderL2);

    cJSON* phasesOrderL3 = cJSON_CreateObject();
    cJSON_AddStringToObject(phasesOrderL3, "name", "phases order L3");
    snprintf(value, sizeof(value), "L%d", regHandle->phasesOrderL3.get());
    cJSON_AddStringToObject(phasesOrderL3, "value", value);
    cJSON_AddItemToArray(root, phasesOrderL3);


    cJSON* dynControlData = cJSON_CreateObject();
    cJSON_AddStringToObject(dynControlData, "name", "dynamic data");
    cJSON_AddItemToArray(root, dynControlData);

    cJSON* evseReserved = cJSON_CreateObject();
    cJSON_AddStringToObject(evseReserved, "name", "evse reserved");
    snprintf(value, sizeof(value), "%d", regHandle->evseReserved.get());
    cJSON_AddStringToObject(evseReserved, "value", value);
    cJSON_AddItemToArray(root, evseReserved);

    cJSON* IL1Max = cJSON_CreateObject();
    cJSON_AddStringToObject(IL1Max, "name", "IL1 max");
    snprintf(value, sizeof(value), "%0.1f A", regHandle->IL1Max.get());
    cJSON_AddStringToObject(IL1Max, "value", value);
    cJSON_AddItemToArray(root, IL1Max);

    cJSON* IL2Max = cJSON_CreateObject();
    cJSON_AddStringToObject(IL2Max, "name", "IL2 max");
    snprintf(value, sizeof(value), "%0.1f A", regHandle->IL2Max.get());
    cJSON_AddStringToObject(IL2Max, "value", value);
    cJSON_AddItemToArray(root, IL2Max);

    cJSON* IL3Max = cJSON_CreateObject();
    cJSON_AddStringToObject(IL3Max, "name", "IL3 max");
    snprintf(value, sizeof(value), "%0.1f A", regHandle->IL3Max.get());
    cJSON_AddStringToObject(IL3Max, "value", value);
    cJSON_AddItemToArray(root, IL3Max);

    cJSON* IL1 = cJSON_CreateObject();
    cJSON_AddStringToObject(IL1, "name", "IL1");
    snprintf(value, sizeof(value), "%0.1f A", regHandle->IL1.get());
    cJSON_AddStringToObject(IL1, "value", value);
    cJSON_AddItemToArray(root, IL1);

    cJSON* IL2 = cJSON_CreateObject();
    cJSON_AddStringToObject(IL2, "name", "IL2");
    snprintf(value, sizeof(value), "%0.1f A", regHandle->IL2.get());
    cJSON_AddStringToObject(IL2, "value", value);
    cJSON_AddItemToArray(root, IL2);

    cJSON* IL3 = cJSON_CreateObject();
    cJSON_AddStringToObject(IL3, "name", "IL3");
    snprintf(value, sizeof(value), "%0.1f A", regHandle->IL3.get());
    cJSON_AddStringToObject(IL3, "value", value);
    cJSON_AddItemToArray(root, IL3);

    cJSON* UL1 = cJSON_CreateObject();
    cJSON_AddStringToObject(UL1, "name", "UL1");
    snprintf(value, sizeof(value), "%0.1f VAC", regHandle->UL1.get());
    cJSON_AddStringToObject(UL1, "value", value);
    cJSON_AddItemToArray(root, UL1);

    cJSON* UL2 = cJSON_CreateObject();
    cJSON_AddStringToObject(UL2, "name", "UL2");
    snprintf(value, sizeof(value), "%0.1f VAC", regHandle->UL2.get());
    cJSON_AddStringToObject(UL2, "value", value);
    cJSON_AddItemToArray(root, UL2);

    cJSON* UL3 = cJSON_CreateObject();
    cJSON_AddStringToObject(UL3, "name", "UL3");
    snprintf(value, sizeof(value), "%0.1f VAC", regHandle->UL3.get());
    cJSON_AddStringToObject(UL3, "value", value);
    cJSON_AddItemToArray(root, UL3);

    cJSON* power = cJSON_CreateObject();
    cJSON_AddStringToObject(power, "name", "power");
    snprintf(value, sizeof(value), "%0.1f W", regHandle->power.get());
    cJSON_AddStringToObject(power, "value", value);
    cJSON_AddItemToArray(root, power);


    cJSON* sessionData = cJSON_CreateObject();
    cJSON_AddStringToObject(sessionData, "name", "charging session data");
    cJSON_AddItemToArray(root, sessionData);

    cJSON* evid = cJSON_CreateObject();
    cJSON_AddStringToObject(evid, "name", "ev id");
    snprintf(value, sizeof(value), "%d", regHandle->evId.get());
    cJSON_AddStringToObject(evid, "value", value);
    cJSON_AddItemToArray(root, evid);

    cJSON* powerUsed = cJSON_CreateObject();
    cJSON_AddStringToObject(powerUsed, "name", "power used in session");
    snprintf(value, sizeof(value), "%d W", regHandle->powerUsed.get());
    cJSON_AddStringToObject(powerUsed, "value", value);
    cJSON_AddItemToArray(root, powerUsed);


    cJSON* debugData = cJSON_CreateObject();
    cJSON_AddStringToObject(debugData, "name", "debug data");
    cJSON_AddItemToArray(root, debugData);

    cJSON* stateLpp = cJSON_CreateObject();
    cJSON_AddStringToObject(stateLpp, "name", "state Lpp");
    snprintf(value, sizeof(value), "%d", regHandle->stateLpp.get());
    cJSON_AddStringToObject(stateLpp, "value", value);
    cJSON_AddItemToArray(root, stateLpp);

    cJSON* state61851 = cJSON_CreateObject();
    cJSON_AddStringToObject(state61851, "name", "state 61851");
    snprintf(value, sizeof(value), "%d", regHandle->state61851.get());
    cJSON_AddStringToObject(state61851, "value", value);
    cJSON_AddItemToArray(root, state61851);

    cJSON* soc = cJSON_CreateObject();
    cJSON_AddStringToObject(soc, "name", "soc");
    snprintf(value, sizeof(value), "%0.1f", regHandle->soc.get());
    cJSON_AddStringToObject(soc, "value", value);
    cJSON_AddItemToArray(root, soc);

    cJSON* tempPlug = cJSON_CreateObject();
    cJSON_AddStringToObject(tempPlug, "name", "temperature plug");
    snprintf(value, sizeof(value), "%0.1f C", regHandle->tempInternalPlug.get());
    cJSON_AddStringToObject(tempPlug, "value", value);
    cJSON_AddItemToArray(root, tempPlug);

    cJSON* tempWallbox = cJSON_CreateObject();
    cJSON_AddStringToObject(tempWallbox, "name", "temperature wallbox");
    snprintf(value, sizeof(value), "%0.1f C", regHandle->tempInternalWallbox.get());
    cJSON_AddStringToObject(tempWallbox, "value", value);
    cJSON_AddItemToArray(root, tempWallbox);

    cJSON* faultCurrentStatus = cJSON_CreateObject();
    cJSON_AddStringToObject(faultCurrentStatus, "name", "fault current status");
    snprintf(value, sizeof(value), "%d", regHandle->faultCurrentStatus.get());
    cJSON_AddStringToObject(faultCurrentStatus, "value", value);
    cJSON_AddItemToArray(root, faultCurrentStatus);

    cJSON* relayStatus = cJSON_CreateObject();
    cJSON_AddStringToObject(relayStatus, "name", "relay status");
    snprintf(value, sizeof(value), "%d", regHandle->relayStatus.get());
    cJSON_AddStringToObject(relayStatus, "value", value);
    cJSON_AddItemToArray(root, relayStatus);
  }

  // Print the generated JSON array
  char* jsonString = cJSON_Print(root);

  uint16_t len = strlen(jsonString);
  httpd_resp_set_type(req, "application/json");  // Set content type to text/plain
  httpd_resp_send(req, jsonString, len);

  // Clean up cJSON objects and the generated string
  cJSON_Delete(root);
  free(jsonString);

  return ESP_OK;

}


esp_err_t httpReqDebugMain(httpd_req_t *req)
{
  /* Send a response */
  const char resp[] = "DEBUG IQ-home";
//  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  char* json = jsonTest();
  uint16_t len = strlen(json);
 httpd_resp_set_type(req, "application/json");  // Set content type to text/plain
  httpd_resp_send(req, json, len);
  return ESP_OK;
}

void taskDebug( void *arg )
{
  //httpd_handle_t server = NULL;
  //ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  auto webServer = Webserver();
  ESP_LOGI(__FILE__, "Tassk debug init done");

  printf("webServer created.");

  webServer.getCbAdd("/debug", httpReqDebugMain);
  webServer.getCbAdd("/info", httpReqInfo);
  webServer.getCbAdd("/livedata", httpReqLivedata);
  printf("webServer URI GET CB created.");

//  auto urlTest2 = Weburi(webServer.server, "/test2", "application/json");
//
//  urlTest2.add(urlTest2.handlerGet);
//  ESP_LOGI(__FILE__, "/test2 added");

  bool done = false;
  while(1)
  {
//    if ((myWebServer != nullptr) && (done == false))
//    {
//      done = true;
//      printf("webServer created.");
//      myWebServer->getCbAdd("/debug", httpReqDebugMain);
//      printf("webServer URI GET CB created.");
//    }

    printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
    printf("DEBUG, free heap size: %d\n", esp_get_free_heap_size());
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }

}

}  // namespace end APP
