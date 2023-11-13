/*------------------------------------------------------------------------------
* webserver.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 9. okt. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <esp_http_server.h>

/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
namespace APP
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/

typedef esp_err_t (*webserver_getCallback_t)(httpd_req_t *r);

class Webserver
{
private:
public:
  static httpd_handle_t server;
  Webserver(void);
  ~Webserver(void);

  void getCbAdd(char const* uri, webserver_getCallback_t cbFunc);
  void getCbRemove(webserver_getCallback_t cbFunc);
  void putCbAdd(char const* uri, webserver_getCallback_t cbFunc);
  void putCbRemove(webserver_getCallback_t cbFunc);
  void optionsCbAdd(char const* uri, webserver_getCallback_t cbFunc) ;
};


class Weburi
{
private:
  static Weburi* instance;
  httpd_uri_t uriGet;

  httpd_handle_t _server;
  char const* _uri;
  char const* _contentType;
public:
  esp_err_t handlerGet(httpd_req_t *req);
  Weburi(httpd_handle_t server, char const* uri, char const* contentType);
  esp_err_t add(webserver_getCallback_t handle);
};

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif

#ifdef __cplusplus
}  // namespace APP
#endif

