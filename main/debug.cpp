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
#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
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

  httpd_resp_send(req, output.data(), output.length());

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

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    printf("DEBUG\n");
  }

}

}  // namespace end APP
