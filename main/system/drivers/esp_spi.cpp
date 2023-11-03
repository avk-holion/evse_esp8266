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

#include "driver/spi.h"
#include "iq_helper.h"
#include "esp_spi.h"
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

   return count;
}

int32_t EspSpiMaster::write(size_t id, void const* pBuffer, size_t bytesToWrite)
{
  return write(id, pBuffer, bytesToWrite, 10000); // TODO max
}

int32_t EspSpiMaster::write(size_t id, void const* pBuffer, size_t bytesToWrite, uint32_t timeOutMs)
{
  uint32_t count = 0;

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


