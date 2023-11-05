/*------------------------------------------------------------------------------
* spi.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 3. nov. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

#include "esp8266/spi_struct.h"
#include "driver/spi.h"
#include "iq_helper.h"
#include "esp_spi.h"

#include "esp_attr.h"
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

/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/

}  // anonymous namespace
/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace DRIVERS
{
EspSpiMaster::EspSpiMaster(spi_host_t host, uint8_t* rxBuffer, uint16_t rxBufferSize)
{
  _handle = 0;
  _config.interface.cpol = 0;
  _config.interface.cpha = 0;
  _config.interface.bit_tx_order = 0;
  _config.interface.bit_rx_order = 0;
  _config.interface.byte_tx_order = 0;
  _config.interface.byte_rx_order = 0;
  _config.interface.mosi_en = 1;
  _config.interface.miso_en = 1;

  _config.intr_enable.val = 0;
  _config.event_cb = nullptr;
  _config.mode = SPI_MASTER_MODE;
  _config.clk_div = SPI_2MHz_DIV;

  _host = host;

  printf("------------ SPI init --------------------\n");
  ESP_ERROR_CHECK(spi_init(_host, &_config));

  /**
   * @note spi_init does not support lower speeds than 2MHz
   * To override the config, we change the pre register.
   * spi_init set it to 0, but if we set it to1 the freq will be the half
   * and 19 = 100kHz
   */
  static DRAM_ATTR spi_dev_t *const SPI[SPI_NUM_MAX] = {&SPI0, &SPI1};
  SPI[_host]->clock.clkdiv_pre = 19; // 2MHz / (19 + 1) = 100kHz

}

size_t EspSpiMaster::open(uint16_t address, INTERFACES::DataStreamConfig_i* config)
{

  return _handle;
}

void EspSpiMaster::close(size_t id)
{

  _handle++;
  if (_handle == 0)
  {
    _handle++;
  }

}

int32_t EspSpiMaster::read(size_t id, void* pBuffer, size_t bytesToRead)
{
  return read(id, pBuffer, bytesToRead, 10000); // TODO MAX
}

int32_t EspSpiMaster::read(size_t id, void* pBuffer, size_t bytesToRead, uint32_t timeOutMs)
{
  uint32_t count = 0;
  uint32_t const* pBuf = reinterpret_cast<uint32_t const*>(pBuffer);

  spi_trans_t trans;
  trans.cmd = nullptr;
  trans.addr = nullptr;
  trans.mosi = nullptr;
  trans.miso = nullptr;
  trans.bits.val = 0;

  trans.miso = (uint32_t*)pBuf;
  trans.bits.miso = bytesToRead * 8;
  printf("SPI transfer RX bytes, %d\n", bytesToRead);
  esp_err_t err = ESP_OK;
  err = spi_trans(_host, &trans);

  printf("SPI transfer done, %d\n", err);
  printf("SPI RX, %d\n", pBuf[0]);

  if (err == ESP_OK)
  {
    count = bytesToWrite;
  }
  else
  {
    ESP_ERROR_CHECK(err);
  }
  return count;
}

int32_t EspSpiMaster::write(size_t id, void const* pBuffer, size_t bytesToWrite)
{
  return write(id, pBuffer, bytesToWrite, 10000); // TODO max
}

int32_t EspSpiMaster::write(size_t id, void const* pBuffer, size_t bytesToWrite, uint32_t timeOutMs)
{
  uint32_t count = 0;
  uint32_t const* pBuf = reinterpret_cast<uint32_t const*>(pBuffer);

  spi_trans_t trans;
  trans.cmd = nullptr;
  trans.addr = nullptr;
  trans.mosi = nullptr;
  trans.miso = nullptr;
  trans.bits.val = 0;

  bytesToWrite += 8;
  trans.mosi = (uint32_t*)pBuf;
  trans.bits.mosi = bytesToWrite * 8;
  printf("SPI transfer TX bytes, %d\n", bytesToWrite);
  esp_err_t err = spi_trans(_host, &trans);
  printf("SPI transfer done, %d\n", err);
  if (err == ESP_OK)
  {
    count = bytesToWrite;
  }
  else
  {
    ESP_ERROR_CHECK(err);
  }

  return count;
}

bool EspSpiMaster::flush(size_t id)
{
  return flush(id, 10000); // TODO max
}

bool EspSpiMaster::flush(size_t id, uint32_t timeOutMs)
{
  bool success = true;

  return success;
}

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
}  // namespace end DRIVERS


