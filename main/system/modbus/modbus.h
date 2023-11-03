/*------------------------------------------------------------------------------
* modbus.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 7. jun. 2023
*
* Generic modbus data types and functions
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include "iq_helper.h"
#include "modbus_register.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
namespace MODBUS
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/

class Modbus
{

protected:

  uint32_t turnArraoundTimeout;    // timeout after a broadcast request from master. [100 - 200 mS]
  uint32_t responseTimeout;        // timeout after a unicast request from master until the slave response [1 - may sec.].

public:
  virtual void openApData(void) = 0;                                    // open communication port
  virtual uint8_t sendApData(uint8_t* pData, uint8_t bytes) = 0;        // send application layer data  to the modbus, adding data link data
  virtual uint8_t receiveApData(uint8_t* pData, uint8_t size) = 0;      // receive application layer data, data link checks are performed.
  virtual void closeApData(void) = 0;                                   // close communication port

  enum class ExceptionCodes : uint8_t
  {
    noException = 0,    // success
    illegalFunction = 1, // this function code is not supported.
    illegalDataAddress = 2, // starting address of data etc.
    illegalDataValue = 3,  // data count or similar exceptions.
    serverDeviceFailure = 4, // if register can not be read for some reason
    ackknowledge = 5,
    serverDeviceBusy = 6,
    memoryParitiError = 8,
    gatewayPathUnavailable = 10,
    gatewayTargetDevieceFaileToResponse = 11,
  };

  Modbus(uint32_t _responseTimeout, uint32_t _turnArraoundTimeout)
  {
    responseTimeout = _responseTimeout;
    turnArraoundTimeout = _turnArraoundTimeout;
  };

};

class ModbusRegArray
{
private:
  uint16_t registerAddressFirst;
  uint16_t registerAddressLast;
  uint8_t registerCount;
  uint16_t* pArray;
  uint8_t readArrayAccessCounter;
  uint8_t writeArrayAccessCounter;
  osMutexId_t mutex;
public:
  ModbusRegArray(uint16_t _registerAddressFirst, uint16_t* _pArray, uint8_t arraySize, osMutexId_t _mutex)
  {
    IQ_ASSERT(_pArray != NULL, "_pArray can not be NULL.");

    registerCount = arraySize / sizeof(pArray[0]);

    registerAddressFirst = _registerAddressFirst;
    registerAddressLast = registerAddressFirst;
    pArray = _pArray;
    mutex = _mutex;
    if (mutex != NULL)
    {
      osMutexAcquire(mutex, osWaitForever);
    }

    if (registerCount > 0)
    {
      registerAddressLast = registerAddressFirst + registerCount - 1;
    }
    else
    {
      IQ_ERROR("arraySize must min be 2 bytes.");
    }

  }

  ~ModbusRegArray()
  {
    if (mutex != NULL)
    {
      osMutexRelease(mutex);
    }
  }

  uint8_t write(ModbusRegister* reg)
  {
    uint8_t byteCount = 0;

    if ((reg->address() >= this->registerAddressFirst) &&
        (reg->address() <= this->registerAddressLast))
    {
      uint8_t index = reg->address() - this->registerAddressFirst;
      uint16_t value;
      value = reg->get();
      this->pArray[index] = __REVSH(value);
      byteCount = sizeof(this->pArray[index]);
    }

    return byteCount;
  }

  uint8_t read(ModbusRegister* reg)
  {
    uint8_t byteCount = 0;

    if ((reg->address() >= this->registerAddressFirst) &&
        (reg->address() <= this->registerAddressLast))
    {
      uint8_t index = reg->address() - this->registerAddressFirst;
      uint16_t value = __REVSH(this->pArray[index]);
      reg->set(value);
      byteCount = sizeof(this->pArray[index]);
    }

    return byteCount;
  }

};

class ErrorCodes
{
public:
  typedef struct __attribute__ ((__packed__))
  {
    uint8_t errorCode;
    uint8_t exceptionCode;
  } responseError_t;
};

class WriteSingleRegister : public ErrorCodes
{
public:
  static const uint8_t functionCode = 0x06;


  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint16_t registerAddress;
    uint16_t registerValue;
  } request_t;
  request_t request;


  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint16_t registerAddress;
    uint16_t registerValue;
  }response_t;

  union __attribute__ ((__packed__))
  {
    response_t data;
    responseError_t error;
  }response;
};

class WriteMultipleRegister : public ErrorCodes
{
public:
  static const uint8_t functionCode = 0x10;


  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint16_t startingAddress;
    uint16_t quantityOfRegisters;
    uint8_t byteCount;
    uint16_t registerValue[123];
  } request_t;
  request_t request;

  typedef struct  __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint16_t startingAddress;
    uint16_t quantityOfRegisters;
  }response_t;

  union __attribute__ ((__packed__))
  {
    response_t data;
    responseError_t error;
  }response;
};

class ReadWriteMultipleRegister : public ErrorCodes
{
public:
  static const uint8_t functionCode = 0x17;


  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint16_t readStartingAddress;
    uint16_t quantityToRead;
    uint16_t writeStartingAddress;
    uint16_t quantityToWrite;
    uint8_t writeByteCount;
    uint16_t writeRegisterValue[121];
  } request_t;
  request_t request;

  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint8_t byteCount;
    uint16_t readRegisterValue[125];
  } response_t;

  union __attribute__ ((__packed__))
  {
    response_t data;
    responseError_t error;
  }response;
};

class ReadHoldingRegister : public ErrorCodes
{
public:
  static const uint8_t functionCode = 0x03;


  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint16_t startingAddress;
    uint16_t quantityOfRegisters;
  } request_t;
  request_t request;

  uint32_t dummy;

  typedef struct __attribute__ ((__packed__))
  {
    uint8_t functionCode;
    uint8_t byteCount;
    uint16_t registerValue[125];
  }response_t;

  union __attribute__ ((__packed__))
  {
    response_t data;
    responseError_t error;
  }response;
};

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace MODBUS
#endif



