/*------------------------------------------------------------------------------
* modbus_serial.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 7. jun. 2023
*
* Generic modbus for SPI bus, simular to serial, but hanldes driver read and write differnt.
* @note only RDU mode (binary) is supported
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>

#include "modbus.h"
#include "data_stream_i.h"
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
class ModbusSPI : public Modbus
{
private:
  INTERFACES::DataStream_i *spi;
  INTERFACES::DataStreamConfig_i *spiConfig;

  static const uint32_t serialTurnArraoundTimeout = 200; // timeout after a broadcast request from master. [100 - 200 mS]
  static const uint32_t serialResponseTimeout = 60000; // TODO not he same for SPI slave as SPI master !!  timeout after a unicast request from master until the slave response [1 - may sec.].
  static const uint16_t crcInitValue = 0xFFFF;

  osMutexId_t mutexCom;
  size_t idCom;

protected:

  uint8_t deviceAddress; //!< address of the modbus (slave) device [1 – 247]
  int16_t modbusCrcCalc(uint8_t* pData, uint16_t dataSize, uint16_t initValue = crcInitValue);
  void openApData(void);
  void closeApData(void);
  uint8_t sendApData(uint8_t* pData, uint8_t bytes) override;      // send application layer data  to the modbus, adding data link data
  uint8_t receiveApData(uint8_t* pData, uint8_t size) override;    // receive application layer data, data link checks are performed.
public:
  ModbusSPI(INTERFACES::DataStream_i* _spi,
               INTERFACES::DataStreamConfig_i* _spiConfig,
               uint32_t _responseTimeout = serialResponseTimeout,
               uint32_t _turnArraoundTimeout = serialTurnArraoundTimeout);
};


/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace MODBUS
#endif



