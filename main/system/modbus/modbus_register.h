/*------------------------------------------------------------------------------
* modbus_register.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 7. jun. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>

#include "iq_helper.h"
#include "observer.h"
#include "cmsis_os2.h"

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
class ModbusRegister : public SYSTEM::Observer
{
protected:
  uint16_t registerAddress;
  int16_t value;
  bool master;

  SYSTEM::Observer::ObserverCb_t observerList[3];

public:
  bool writeable;

  ModbusRegister(uint16_t _address, bool _master, bool _writable = true)
  : Observer(observerList, IQ_ARRAY_SIZE(observerList))
  {
    registerAddress = _address;
    master = _master;
    writeable = _writable;
    value = 0;
  }

  uint16_t address(void)
  {
    return registerAddress;
  }

  void set(int16_t value)
  {
    this->value = value;
    void* data = &value;
    notify(data);
  };

  int16_t get(void)
  {
    return this->value;
  }
};

class ModbusRegisterFloat
{
private:
  ModbusRegister* low;
  ModbusRegister* high;
  osMutexId_t mutex;

  typedef union
  {
    struct
    {
      uint16_t high;
      uint16_t low;
    }reg;
    float value;
  }regFloat_t;
public:
  ModbusRegisterFloat(ModbusRegister* _low, ModbusRegister* _high, osMutexId_t _mutex = NULL)
  {
    high = _high;
    low = _low;
    mutex = _mutex;
  }


  void set(float _value)
  {
    /**
     * split the float value up into two 16 bit values
     */
    regFloat_t value;

    if (mutex != NULL)
    {
      osMutexAcquire(mutex, osWaitForever);
    }
    value.value = _value;

    low->set(value.reg.low);
    high->set(value.reg.high);
    if (mutex != NULL)
    {
      osMutexRelease(mutex);
    }
  };

  float get(void)
  {
    /**
     * merge the two 16 bit values into one float
     */
    regFloat_t value;

    if (mutex != NULL)
    {
      osMutexAcquire(mutex, osWaitForever);
    }
    value.reg.low = low->get();
    value.reg.high = high->get();
    if (mutex != NULL)
    {
      osMutexRelease(mutex);
    }
    return value.value;
  }
};

class ModbusRegisterUint32
{
private:
  ModbusRegister* low;
  ModbusRegister* high;
  osMutexId_t mutex;

  typedef union
  {
    struct
    {
      uint16_t high;
      uint16_t low;
    }reg;
    uint32_t value;
  }regUint32_t;

public:
  ModbusRegisterUint32(ModbusRegister* _low, ModbusRegister* _high, osMutexId_t _mutex = NULL)
  {
    low = _low;
    high = _high;
    mutex = _mutex;
  }


  void set(uint32_t _value)
  {
    /**
     * split the uint32_t value up into two 16 bit values
     */
    regUint32_t value;

    value.value = _value;

    if (mutex != NULL)
    {
      osMutexAcquire(mutex, osWaitForever);
    }
    low->set(value.reg.low);
    high->set(value.reg.high);

    if (mutex != NULL)
    {
      osMutexRelease(mutex);
    }
  };

  uint32_t get(void)
  {
    /**
     * merge the two 16 bit values into one uint32_t
     */
    regUint32_t value;
    if (mutex != NULL)
    {
      osMutexAcquire(mutex, osWaitForever);
    }
    value.reg.low = low->get();
    value.reg.high = high->get();

    if (mutex != NULL)
    {
      osMutexRelease(mutex);
    }

    return value.value;
  }
};

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace MODBUS
#endif



