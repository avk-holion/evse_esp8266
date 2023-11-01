/*------------------------------------------------------------------------------
* udpDaemon.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 31. okt. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include "udpDaemon.h"

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <lwip/sockets.h>
#include <arpa/inet.h>

#include "esp_log.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#define UDPD_MAX_STRING_LENGTH 40
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
static char udpSearchString[UDPD_MAX_STRING_LENGTH] = "";      // hold the string to search for on the UDP port
static char udpResponseString[UDPD_MAX_STRING_LENGTH] = "";    // hold the string to send, when a key is received.
static int udpPort; // the port number to use.
/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/

}  // anonymous namespace

/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace APP
{

class UdpSocket
{
private:
  sockaddr_in _socketAddrIn;
  sockaddr_in _socketAddrOut;

public:
  static int sockfd;

  UdpSocket(int port);
  ssize_t read(uint8_t* dataBuffer, ssize_t length);
  ssize_t write(uint8_t* dataBuffer, ssize_t length);
};

int UdpSocket::sockfd = 0;

UdpSocket::UdpSocket(int port)
{
  if (sockfd == 0)
  {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
      ESP_LOGE(__FILE__, "UDP socket creation failed!");
    }
  }

  if (sockfd > 0)
  {
    // Set up the server address structure
    memset(&_socketAddrIn, 0, sizeof(_socketAddrIn));
    memset(&_socketAddrOut, 0, sizeof(_socketAddrOut));
    _socketAddrIn.sin_family = AF_INET;
    _socketAddrIn.sin_port = htons(port);
    _socketAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

    _socketAddrOut.sin_family = AF_INET;
    _socketAddrOut.sin_port = htons(port); // important to set out port number
    _socketAddrOut.sin_addr.s_addr = htonl(INADDR_ANY);


    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr*) &_socketAddrIn, sizeof(_socketAddrIn)) < 0)
    {
      ESP_LOGE(__FILE__, "UDP socket bind failed!");
      close(sockfd);
      sockfd = 0;
    }
  }

}

ssize_t UdpSocket::read(uint8_t* dataBuffer, ssize_t length)
{
  ssize_t count = 0;

  // Buffer to receive incoming data
  memset(dataBuffer, 0, length);

  // Receive data from clients
  socklen_t clientAddrLen = sizeof(_socketAddrIn);
  count = recvfrom(sockfd, dataBuffer, length, 0, (struct sockaddr*) &_socketAddrIn, &clientAddrLen);
  if (count < 0)
  {
    ESP_LOGI(__FILE__, "UDP Error receiving data!");
  }
  else
  {
    ESP_LOGI(__FILE__, "UDP Received data from %s: port: %d", inet_ntoa(_socketAddrIn.sin_addr), ntohs(_socketAddrOut.sin_port));
  }

  return count;
}

ssize_t UdpSocket::write(uint8_t* dataBuffer, ssize_t length)
{
  ssize_t count = 0;

  _socketAddrOut.sin_addr = _socketAddrIn.sin_addr;
  count = sendto(sockfd, dataBuffer, length, 0, (struct sockaddr*) &_socketAddrOut, sizeof(_socketAddrOut));

  // Send data to the server
  if (count != length)
  {
    ESP_LOGI(__FILE__, "UDP Error sending data!");
  }
  else
  {
    ESP_LOGI(__FILE__, "UDP Data sent successfully to %s: port: %d", inet_ntoa(_socketAddrOut.sin_addr), ntohs(_socketAddrOut.sin_port));

  }

  return count;
}


void taskUdpDaemon(void* arg)
{

  while (udpPort == 0)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  auto udpSocket = UdpSocket(udpPort);

  uint8_t dataBuffer[UDPD_MAX_STRING_LENGTH] = {0};

  while (1)
  {
    ESP_LOGI(__FILE__, "waiting for UDP packet!");
    udpSocket.read(dataBuffer, sizeof(dataBuffer));
    size_t count = strcspn(udpSearchString, "\n");
    count--;

    if (memcmp(dataBuffer, udpSearchString, count) == 0)
    {
      size_t txLen = strlen(udpResponseString) - 1;
      udpSocket.write((uint8_t*)udpResponseString, txLen);
    }
    else
    {
      ESP_LOGI(__FILE__, "wrong search key: %s, expected: %s", dataBuffer, udpSearchString);
    }

    printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
  }
}

void UdpDaemon_broadcastStart(int port, char const* keyString, char const* responseString)
{
  if (port > 0)
  {
    udpPort = port;

    strcpy(udpSearchString, keyString);
    strcpy(udpResponseString, responseString);
  }
}

}  // namespace end APP
