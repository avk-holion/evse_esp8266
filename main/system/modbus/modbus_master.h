/*------------------------------------------------------------------------------
* modbus_master.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 7. jun. 2023
*
* Modbus master class, to handle modbus communication from master to slave.
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>

#include "modbus.h"
#include "modbus_register.h"
#include "modbus_spi.h"
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

class ModbusMaster : public ModbusSPI
{
private:
  uint8_t retries;
  osMutexId_t mutex;

  Modbus::ExceptionCodes isError(ErrorCodes::responseError_t* pData, uint8_t size);
  Modbus::ExceptionCodes requestTransfer(void* pTxData, uint8_t txSize, void* pRxData, uint8_t rxSize);

public:
  ModbusMaster (INTERFACES::DataStream_i *comport,
                INTERFACES::DataStreamConfig_i *comportConfig,
                uint32_t _responseTimeout = 1000,
                uint32_t _turnArraoundTimeout = 100,
                uint8_t _retries = 1,
                osMutexId_t _mutex = NULL);



  Modbus::ExceptionCodes writeSingleRegister(ModbusRegister* reg);
  Modbus::ExceptionCodes writeMultipleRegister(ModbusRegister** regArray, uint8_t arrayCount);
  Modbus::ExceptionCodes readWriteMultipleRegister(ModbusRegister** readRegArray, uint8_t readArrayCount, ModbusRegister** writeRegArray, uint8_t writeArrayCount);
  Modbus::ExceptionCodes readHoldingRegister(ModbusRegister** regArray, uint8_t arrayCount);
};

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace MODBUS
#endif



