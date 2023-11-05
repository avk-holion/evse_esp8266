/*------------------------------------------------------------------------------
* evse.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 2. nov. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

#include "evse.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include "modbusWallbox.h"
#include "modbus.h"
#include "modbus_master.h"
#include "esp_spi.h"


/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/

namespace  // anonymous
{
/*------------------------------------------------------------------------------
                        Constant declarations
------------------------------------------------------------------------------*/
const osMutexAttr_t MutexEvseComModbusAttr =
{
  "mtxEvseComModbus",                       // human readable mutex name
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
static MODBUS::ModbusMaster* modbusMasterEvse = NULL;
static WALLBOX_API::ModbusWallbox* evseReg;

/*------------------------------------------------------------------------------
                        Local functions and classes
------------------------------------------------------------------------------*/

}  // anonymous namespace

/*------------------------------------------------------------------------------
            classes and function declared in primary header file
------------------------------------------------------------------------------*/
namespace APP
{
/*------------------------------------------------------------------------------
*
* Init the modbus registers to default values
*
* ----------------------------------------------------------------------------*/
static void evseInit(void)
{
  evseReg->wallboxRegisterMapVer.set(0);
  evseReg->hwRevPlug.set(0);
  evseReg->hwRevWbPower.set(0);
  evseReg->hwRevWbMcu.set(0);
  evseReg->swRevPlug.set(0);
  evseReg->swRevWallbox.set(0);
  evseReg->statusSelftestPlug.set(0);
  evseReg->statusSelftestWallbox.set(0);
  evseReg->currentPhasesMaxPlug.set(0);
  evseReg->currentPhasesMaxWallbox.set(0);

  evseReg->serialNumber.set(0);
  evseReg->currentMaxInstallation.set(0);
  evseReg->phasesInstallation.set(0);
  evseReg->phasesOrderL1.set(0);
  evseReg->phasesOrderL2.set(0);
  evseReg->phasesOrderL3.set(0);
  evseReg->saveData.set(0);

  evseReg->evseReserved.set(0);
  evseReg->IL1Max.set(0.0);
  evseReg->IL2Max.set(0.0);
  evseReg->IL3Max.set(0.0);
  evseReg->IL1.set(0.0);
  evseReg->IL2.set(0.0);
  evseReg->IL3.set(0.0);
  evseReg->UL1.set(0.0);
  evseReg->UL2.set(0.0);
  evseReg->UL3.set(0.0);
  evseReg->power.set(0.0);

  evseReg->evId.set(0);
  evseReg->powerUsed.set(0);

  evseReg->stateLpp.set(0);
  evseReg->state61851.set(0);
  evseReg->soc.set(0.0);
  evseReg->tempInternalPlug.set(0.0);
  evseReg->tempInternalWallbox.set(0.0);
  evseReg->faultCurrentStatus.set(0);
  evseReg->relayStatus.set(0);

  evseReg->plugNewSwRev.set(0);
  evseReg->bootActivate.set(0);
}

WALLBOX_API::ModbusWallbox* evseRegisterHandleGet()
{
  return evseReg;
}

void taskEvse( void *arg )
{

  osMutexId_t mutexModbusData;
  mutexModbusData = osMutexNew(&MutexEvseComModbusAttr);

  WALLBOX_API::ModbusWallbox modusEvseReg = WALLBOX_API::ModbusWallbox();
  evseReg = &modusEvseReg;

  uint8_t rxBuffer[200];

  auto spiHandle = DRIVERS::EspSpiMaster(HSPI_HOST, rxBuffer, sizeof(rxBuffer));
  INTERFACES::DataStream_i* handle = &spiHandle;
  INTERFACES::DataStreamConfig_i config;

  // @NOTE response = e.g. 10 mS will fail, since it times out before finish
  auto modbus = MODBUS::ModbusMaster(handle, &config, 20, 200, 2, mutexModbusData);
  modbusMasterEvse = &modbus;

  evseInit();

  MODBUS::ModbusRegister* regArray[] =
      {
          &evseReg->stateLpp,
          &evseReg->state61851,
      };

  uint16_t regArrayLen = IQ_ARRAY_SIZE(regArray);

  ESP_LOGI(__FILE__, "Task EVSE init done");
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  while(1)
  {
    modbusMasterEvse->readHoldingRegister(regArray, regArrayLen);

    printf("Task \"%s\" remaining stack size: %u bytes\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

}  // namespace end APP
