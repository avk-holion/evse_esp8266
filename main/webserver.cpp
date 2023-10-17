/*------------------------------------------------------------------------------
* webserver.cpp
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

#include "esp_log.h"


#include "webserver.h"
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
esp_err_t get_handler(httpd_req_t *req);


/*------------------------------------------------------------------------------
                        Variable declarations
------------------------------------------------------------------------------*/
/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/test",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};
/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/
esp_err_t get_handler(httpd_req_t *req)
{
  // ESP_LOGI(__FILE, "URI: %s", req->uri);
  printf("******** URI: %s", req->uri);
    /* Send a simple response */
    //httpd_resp_set_type(req, "text/plain");  // Set content type to text/plain
    const char resp[] = "{\n\"state\": \"serial-required\"\n}";
     httpd_resp_set_type(req, "application/json");  // Set content type to text/plain
    httpd_resp_send(req, resp, sizeof(resp));
    return ESP_OK;
}

// Er nød til at lave et arry af alle opbejcter for URI, således man kan kalde eller få output/content fra dem hver især.



}  // anonymous namespace

/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace APP
{

Weburi* Weburi::instance;

Weburi::Weburi(httpd_handle_t server, char const* uri, char const* contentType)
{
  _server = server;
  _uri = uri;
  _contentType = contentType;

  uriGet.uri = this->_uri;
  uriGet.method = HTTP_GET;


//  auto funcPtr = [this](httpd_req_t* req) {this->handlerGet(req);
//  uriGet.handler = funcPtr;
  // uriGet.handler = reinterpret_cast<esp_err_t (*)(httpd_req_t *r)>(handlerGet);
  uriGet.user_ctx = NULL;
}

esp_err_t Weburi::handlerGet(httpd_req_t *req)
{
  esp_err_t status;
  /* Send a simple response */

  const char resp[] = "{\n\"state\": \"serial-required_CLASS\"\n}";
  status = httpd_resp_set_type(req, _contentType);  // Set content type to text/plain
  if (status == ESP_OK)
  {
    status = httpd_resp_send(req, resp, sizeof(resp));
  }

  return ESP_OK;
}

esp_err_t Weburi::add(webserver_getCallback_t handle)
{
  uriGet.handler = handle;
  return httpd_register_uri_handler(_server, &uriGet);
}

httpd_handle_t Webserver::server = NULL;

Webserver::Webserver(void)
{
  if (server == NULL) // only support one server at the time
  {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(__FILE__, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        getCbAdd("/", get_handler);
        // httpd_register_uri_handler(server, &uri_get);
        ESP_LOGI(__FILE__, "Starting ok.");
    }
    else
    {
      ESP_LOGE(__FILE__, "Error starting server!");
    }
  }

}

Webserver::~Webserver(void)
{
  if (server != NULL)
  {
    httpd_stop(server);
  }
}

void Webserver::getCbAdd(char const* uri, webserver_getCallback_t cbFunc)
{
  httpd_uri_t getHandler =
      {
      .uri       = uri,
      .method    = HTTP_GET,
      .handler   = cbFunc,
      /* Let's pass response string in user
       * context to demonstrate it's usage */
      .user_ctx  = NULL
  };

  ESP_LOGI("WEB", "Registering URI GET handlers.");
  if (httpd_register_uri_handler(server, &getHandler) != ESP_OK)
  {
    ESP_LOGI("WEB", "FAILED");
  }
  else
  {
    ESP_LOGI("WEB", "OK");
  }
}

void Webserver::getCbRemove(webserver_getCallback_t cbFunc)
{

}

void Webserver::putCbAdd(char const* uri, webserver_getCallback_t cbFunc)
{
  httpd_uri_t putHandler =
      {
      .uri       = uri,
      .method    = HTTP_PUT,
      .handler   = cbFunc,
      /* Let's pass response string in user
       * context to demonstrate it's usage */
      .user_ctx  = NULL
  };

  ESP_LOGI("WEB", "Registering URI PUT handlers.");
  if (httpd_register_uri_handler(server, &putHandler) != ESP_OK)
  {
    ESP_LOGI("WEB", "FAILED");
  }
  else
  {
    ESP_LOGI("WEB", "OK");
  }
}

void Webserver::putCbRemove(webserver_getCallback_t cbFunc)
{

}

}  // namespace end APP
