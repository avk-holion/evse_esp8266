/*------------------------------------------------------------------------------
* modbus_spi.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 7. jun. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <cmsis_os2.h>
#include <string.h>
#include <modbus/modbus_spi.h>


#include "iq_helper.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/

namespace  // anonymous
{
/*------------------------------------------------------------------------------
                        Constant declarations
------------------------------------------------------------------------------*/

const osMutexAttr_t MutexModbusSPIAttr =
{
  "mtxModbusSPI",                           // human readable mutex name
  osMutexRecursive | osMutexPrioInherit,    // attr_bits
  NULL,                                     // memory for control block
  0U                                        // size for control block
};

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
namespace MODBUS
{


/**-----------------------------------------------------------------------------
 * Calculate CRC for a given data array. Please note, if CRC is added to the
 * array, the result will be 0x0000.
 *
 * MODBUS crc 16 calculator. Uses polynomial = 1 + x2 + x 15 + x 16.
 * Init value must be set to 0xFFFF.
 *
 * @param[in] pData Pointer to the data array.
 * @param[in] dataSize size of the data array in bytes.
 * @return The calculated CRC for the data array
 * ---------------------------------------------------------------------------*/
int16_t ModbusSPI::modbusCrcCalc (uint8_t *pData, uint16_t dataSize, uint16_t initValue)
{
  size_t i;
  int16_t result = 0;

  /* MODBUS CRC initial value is 0xFFFF. */
  uint16_t crc = initValue;
  for(i = 0; i < dataSize; i++)
  {
    size_t n;
    crc ^= (uint16_t)pData[i];
    for (n = 0; n < 8; ++n)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ 0xA001;
      }
      else
      {
        crc = (crc >> 1);
      }
    }
    result = crc;
  }

  return result;
}

void ModbusSPI::openApData(void)
{
  osMutexAcquire(mutexCom, osWaitForever);

  idCom = this->spi->open(0, this->spiConfig);
}

void ModbusSPI::closeApData(void)
{
  this->spi->close(idCom);

  osMutexRelease(mutexCom);
}

uint8_t ModbusSPI::sendApData (uint8_t *pData, uint8_t bytes)
{
  int16_t txDataCount = 0;

  IQ_ASSERT(pData != NULL, "pData can not be null.");

  if (bytes > 253)
  {
    IQ_ERROR("Modbus application layer data can not hold more than 253 bytes, %d", bytes);
  }
  else
  {
    uint8_t txBuffer[256]; // max size of data on modbus is 256 including address and CRC
    txDataCount = 1 + bytes;

    txBuffer[0] = 1 + bytes + 2;

    uint8_t* dst = &txBuffer[1];
    uint8_t* src = pData;
    memcpy(dst, src, bytes);

    // add crc on txBuffer
    uint16_t crc = modbusCrcCalc(txBuffer, txDataCount);
    memcpy(&(txBuffer[txDataCount]), &crc, sizeof(crc));
    txDataCount += sizeof(crc);

    txDataCount = this->spi->write(idCom, txBuffer, txDataCount);

    txDataCount -= 1 + sizeof(crc);
    if (txDataCount < 0)
    {
      txDataCount = 0;
    }
  }

  return txDataCount;
}


uint8_t ModbusSPI::receiveApData (uint8_t *pData, uint8_t size)
{
  IQ_ASSERT(pData != NULL, "pData can not be null.");

  uint8_t rxBuffer[256]; // max size of data on modbus is 256 including address and CRC
  int32_t rxDataCount = 0;

  // get the size of the package
  rxDataCount = this->spi->read(idCom, rxBuffer, 1, this->responseTimeout);

  if (rxDataCount == 1)
  {
    if (rxBuffer[0] <= 2)
    {
      IQ_INFO("modbus SPI can not receive less than 2 bytes");
    }
    else if (rxBuffer[0] > 253)
    {
      IQ_INFO("modbus SPI can not receive higher than 253 bytes");
    }
    else
    {
      // IQ_INFO("SPI length: %d", rxBuffer[0]);

      uint8_t dataCount;
      dataCount = rxBuffer[0] - 1;

      rxDataCount = this->spi->read(idCom, &rxBuffer[1], dataCount, this->responseTimeout);

      if (rxDataCount != dataCount)
      {
        IQ_INFO("Modbus SPI can not received the needed bytes %d/%d", rxDataCount, dataCount);
      }
      else
      {
  //      rxDataCount = 7;
  //      //03, 00, 01, 00, 02, 0x95, 0x52};
  //      rxBuffer[1] = 3;
  //      rxBuffer[2] = 0;
  //      rxBuffer[3] = 1;
  //      rxBuffer[4] = 0;
  //      rxBuffer[5] = 2;
  //      rxBuffer[6] = 0x95;
  //      rxBuffer[7] = 0x52;

        // new data received from, verify it
        uint16_t crc;
        crc = this->modbusCrcCalc(rxBuffer, rxDataCount + 1);
        rxDataCount -= 2;

        if (crc != 0)
        {
          IQ_INFO("modbus SPI received wrong CRC, byte count: %d.", (int)rxDataCount);
          rxDataCount = 0;
        }
        else
        {
          // data is without errors
          memcpy(pData, &rxBuffer[1], rxDataCount);
        }
      }
    }

  }
  else
  {
    rxDataCount = 0;
  }

  return rxDataCount;
}

ModbusSPI::ModbusSPI (INTERFACES::DataStream_i* _spi,
                            INTERFACES::DataStreamConfig_i* _spiConfig,
                            uint32_t _responseTimeout,
                            uint32_t _turnArraoundTimeout)
: Modbus(_responseTimeout, _turnArraoundTimeout)
{
  IQ_ASSERT(_spi != NULL, "spi can not be NULL.");
  IQ_ASSERT(_spiConfig != NULL, "spiConfig can not be NULL.");

  this->spi = _spi;
  this->spiConfig = _spiConfig;

  mutexCom = osMutexNew(&MutexModbusSPIAttr);
}

}  // namespace end MODBUS
