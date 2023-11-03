/*------------------------------------------------------------------------------
* dataStream_i.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: Oct 20, 2022
*
* Interface used to transfer data on communication channels or in flash etc.
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
namespace INTERFACES
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/
class DataStreamConfig_i
{

};

class DataStream_i
{
protected:
  uint16_t address;
  DataStreamConfig_i* config;
  uint32_t readCounter;
  size_t bytesToRead;
  uint32_t writeCounter;
  size_t bytesToWrite;
public:
  virtual size_t open(uint16_t address, DataStreamConfig_i* config) = 0;
  virtual void close(size_t id) = 0;
  virtual int32_t read(size_t id, void* pBuffer, size_t bytesToRead) = 0;
  virtual int32_t read(size_t id, void* pBuffer, size_t bytesToRead, uint32_t timeOutMs) = 0;
  virtual int32_t write(size_t id, void const* pBuffer, size_t bytesToWrite) = 0;
  virtual int32_t write(size_t id, void const* pBuffer, size_t bytesToWrite, uint32_t timeOutMs) = 0;
  virtual bool flush(size_t id) = 0;
  virtual bool flush(size_t id, uint32_t timeOutMs) = 0;
};

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif



#ifdef __cplusplus
}  // namespace MY_NAMESPACE
#endif



