/*------------------------------------------------------------------------------
* esp_spi.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 3. nov. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <driver/spi.h>
#include <stdint.h>
#include "data_stream_i.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
namespace DRIVERS
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/
class EspSpiMasterConfig : public INTERFACES::DataStreamConfig_i
{

};

class EspSpiMaster : public INTERFACES::DataStream_i
{
private:
  spi_host_t _host;
  size_t _handle;
public:
  EspSpiMaster(spi_host_t host, uint8_t* rxBuffer, uint16_t rxBufferSize);
  ~EspSpiMaster() {};

  size_t open(uint16_t address, INTERFACES::DataStreamConfig_i* config) override;
  void close(size_t id) override;
  int32_t read(size_t id, void* pBuffer, size_t bytesToRead) override;
  int32_t read(size_t id, void* pBuffer, size_t bytesToRead, uint32_t timeOutMs) override;
  int32_t write(size_t id, void const* pBuffer, size_t bytesToWrite) override;
  int32_t write(size_t id, void const* pBuffer, size_t bytesToWrite, uint32_t timeOutMs) override;
  bool flush(size_t id) override;
  bool flush(size_t id, uint32_t timeOutMs) override;
};
/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace DRIVERS
#endif



