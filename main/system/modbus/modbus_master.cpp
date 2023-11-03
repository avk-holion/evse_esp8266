/*------------------------------------------------------------------------------
* modbus_master.cpp
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

#include "iq_helper.h"
#include "modbus_master.h"
#include "modbus.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

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
namespace MODBUS
{

Modbus::ExceptionCodes ModbusMaster::isError(ErrorCodes::responseError_t* pData, uint8_t size)
{
  Modbus::ExceptionCodes exception = Modbus::ExceptionCodes::noException;

  if (size == sizeof(*pData))
  {
    if (pData->errorCode & 0x80)
    {
      exception = static_cast<Modbus::ExceptionCodes>(pData->exceptionCode);
      IQ_INFO("Modbus execption %d", pData->exceptionCode);
    }
  }

  return exception;
};

/*------------------------------------------------------------------------------
* Handles unicast transfer of data to a modbus slave.
* SEnd the request data, an receive the repose.
* Error check and retires is handled too.
*
* @param[in]    pTxData pointer to app layer data to send.
* @param[in]    txSize size of data to send.
* @param[out]   pRxData pointer to hold incoming data from slave.
* @param[in]    rxSize number of data to receive from slave.
*
* @retval ExceptionCodes
* ----------------------------------------------------------------------------*/
Modbus::ExceptionCodes ModbusMaster::requestTransfer(void* pTxData, uint8_t txSize, void* pRxData, uint8_t rxSize)
{
  Modbus::ExceptionCodes exception = Modbus::ExceptionCodes::serverDeviceFailure;

  uint8_t txCount;
  uint8_t rxCount = 0;
  uint8_t retry;

  for (retry = 0; retry <= this->retries; retry++)
  {
    this->openApData();

    rxCount = 0;
    txCount = this->sendApData((uint8_t*)pTxData, txSize);

    if (txCount == txSize)
    {
      rxCount = this->receiveApData((uint8_t*)pRxData, rxSize);
    }

    this->closeApData();

    if (rxCount >= sizeof(ErrorCodes::responseError_t))
    {
      exception = this->isError((ErrorCodes::responseError_t*)pRxData, rxCount);
      break;
    }
  }

  return exception;
}


/**
 * PUBLIC
 */
ModbusMaster::ModbusMaster(INTERFACES::DataStream_i* comport,
                                           INTERFACES::DataStreamConfig_i* comportConfig,
                                           uint32_t _responseTimeout,
                                           uint32_t _turnArraoundTimeout,
                                           uint8_t _retries,
                                           osMutexId_t _mutex)
: MODBUS::ModbusSPI(comport, comportConfig, _responseTimeout, _turnArraoundTimeout)
{
  retries = _retries;
  mutex = _mutex;
}


Modbus::ExceptionCodes ModbusMaster::writeSingleRegister(ModbusRegister* reg)
{
  Modbus::ExceptionCodes exception = Modbus::ExceptionCodes::serverDeviceFailure;

  MODBUS::WriteSingleRegister obj;

  obj.request.functionCode = obj.functionCode;
  obj.request.registerAddress = __REVSH(reg->address());
  obj.request.registerValue = __REVSH(reg->get());

  exception = requestTransfer(&obj.request, sizeof(obj.request), &obj.response, sizeof(obj.response));

  return exception;
}

Modbus::ExceptionCodes ModbusMaster::writeMultipleRegister(ModbusRegister** regArray, uint8_t arrayCount)
{
  IQ_ASSERT(regArray != NULL, "pointer can not be NULL");
  IQ_ASSERT(arrayCount > 0, "arrayCount must at least be 1");
  IQ_ASSERT(arrayCount <= 123, "arrayCount must max  be 123");

  Modbus::ExceptionCodes exception = Modbus::ExceptionCodes::serverDeviceFailure;

  MODBUS::WriteMultipleRegister obj;

  obj.request.functionCode = obj.functionCode;
  obj.request.startingAddress = __REVSH(regArray[0]->address());
  obj.request.quantityOfRegisters = __REVSH(arrayCount);
  obj.request.byteCount = arrayCount * 2;

  /**
   * add register values into array
   */
  IQ_ASSERT(((uint32_t)&obj.request.registerValue[0] % 2) == 0, "obj.response.registerValue must align on even address.");
  auto array = ModbusRegArray(regArray[0]->address(), obj.request.registerValue, obj.request.byteCount, mutex);

  uint8_t byteCount = 0;
  size_t n;
  for(n = 0; n < arrayCount; n++)
  {
    auto pReg = regArray[n];
    byteCount += array.write(pReg);
  }

  if (byteCount != obj.request.byteCount)
  {
    IQ_ERROR("Only %d bytes transfered to array. Expected %d bytes.", byteCount, obj.request.byteCount);
  }
  else
  {
    // ready to send data
    uint8_t txSize = sizeof(obj.request) - sizeof(obj.request.registerValue);
    txSize += obj.request.byteCount;

    exception = requestTransfer(&obj.request, txSize, &obj.response, sizeof(obj.response));
  }

  return exception;
}

Modbus::ExceptionCodes ModbusMaster::readWriteMultipleRegister(ModbusRegister** readRegArray, uint8_t readArrayCount, ModbusRegister** writeRegArray, uint8_t writeArrayCount)
{
  IQ_ASSERT(readRegArray != NULL, "pointer can not be NULL");
  IQ_ASSERT(readArrayCount > 0, "readArrayCount must at least be 1");
  IQ_ASSERT(readArrayCount <= 125, "readArrayCount must max be 125");
  IQ_ASSERT(writeRegArray != NULL, "pointer can not be NULL");
  IQ_ASSERT(writeArrayCount > 0, "writeArrayCount must at least be 1");
  IQ_ASSERT(writeArrayCount <= 121, "writeArrayCount must max  be 121");

  Modbus::ExceptionCodes exception = Modbus::ExceptionCodes::serverDeviceFailure;

  MODBUS::ReadWriteMultipleRegister obj;

  obj.request.functionCode = obj.functionCode;
  obj.request.readStartingAddress = __REVSH(readRegArray[0]->address());
  obj.request.quantityToRead = __REVSH(readArrayCount);
  obj.request.writeStartingAddress = __REVSH(writeRegArray[0]->address());
  obj.request.quantityToWrite = __REVSH(writeArrayCount);
  obj.request.writeByteCount = writeArrayCount * 2;

  /**
   * add register values into array
   */
  IQ_ASSERT(((uint32_t)&obj.request.writeRegisterValue[0] % 2) == 0, "obj.response.writeRegisterValue must align on even address.");
  auto array = ModbusRegArray(writeRegArray[0]->address(), obj.request.writeRegisterValue, obj.request.writeByteCount, mutex);

  uint8_t byteCount = 0;
  size_t n;
  for(n = 0; n < writeArrayCount; n++)
  {
    auto pReg = writeRegArray[n];
    byteCount += array.write(pReg);
  }


  if (byteCount != obj.request.writeByteCount)
  {
    IQ_ERROR("Only %d bytes transfered to array. Expected %d bytes.", byteCount, obj.request.writeByteCount);
  }
  else
  {
    // ready to send data
    uint8_t txSize = sizeof(obj.request) - sizeof(obj.request.writeRegisterValue);
    txSize += obj.request.writeByteCount;

    uint8_t rxSize = sizeof(obj.response) - sizeof(obj.response.data.readRegisterValue);
    rxSize += readArrayCount * 2;

    exception = requestTransfer(&obj.request, txSize, &obj.response, rxSize);

    if (exception == Modbus::ExceptionCodes::noException)
    {
      // response received ok, load register value data into register object
      IQ_ASSERT(((uint32_t)&obj.response.data.readRegisterValue[0] % 2) == 0, "obj.response.readRegisterValue must align on even address.");
      auto array = ModbusRegArray(readRegArray[0]->address(), obj.response.data.readRegisterValue, obj.response.data.byteCount, mutex);

      uint8_t byteCount = 0;
      size_t n;
      for(n = 0; n < readArrayCount; n++)
      {
        auto pReg = readRegArray[n];
        byteCount += array.read(pReg);
      }

      if (byteCount != obj.response.data.byteCount)
      {
        IQ_ERROR("Only %d bytes transfered to registers. Expected %d bytes.", byteCount, obj.response.data.byteCount);
      }
    }
  }

  return exception;
}

Modbus::ExceptionCodes ModbusMaster::readHoldingRegister(ModbusRegister** regArray, uint8_t arrayCount)
{
  IQ_ASSERT(regArray != NULL, "pointer can not be NULL");
  IQ_ASSERT(arrayCount > 0, "arrayCount must at least be 1");
  IQ_ASSERT(arrayCount <= 125, "arrayCount must max be 125");

  Modbus::ExceptionCodes exception = Modbus::ExceptionCodes::serverDeviceFailure;

  MODBUS::ReadHoldingRegister obj;

  obj.request.functionCode = obj.functionCode;
  obj.request.startingAddress = __REVSH(regArray[0]->address());
  obj.request.quantityOfRegisters = __REVSH(arrayCount);

  uint8_t rxSize = sizeof(obj.response) - sizeof(obj.response.data.registerValue);
  rxSize += arrayCount * 2;

  exception = requestTransfer(&obj.request, sizeof(obj.request), &obj.response, rxSize);

  if (exception == Modbus::ExceptionCodes::noException)
  {
    // response received ok, load register value data into register object

    IQ_ASSERT(((uint32_t)&obj.response.data.registerValue[0] % 2) == 0, "obj.response.data.registerValue must align on even address.");
    auto array = ModbusRegArray(regArray[0]->address(), obj.response.data.registerValue, obj.response.data.byteCount, mutex);

    uint8_t byteCount = 0;
    size_t n;
    for(n = 0; n < arrayCount; n++)
    {
      auto pReg = regArray[n];
      byteCount += array.read(pReg);
    }

    if (byteCount != obj.response.data.byteCount)
    {
      IQ_ERROR("Only %d bytes transfered to registers. Expected %d bytes.", byteCount, obj.response.data.byteCount);
    }
  }

  return exception;
}

}  // namespace end MODBUS
