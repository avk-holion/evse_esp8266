/*------------------------------------------------------------------------------
* modbusWallbox.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: Aug 15, 2023
*
* This file define the API to the STM32 in the wallbox.
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>

#include "modbus_register.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
namespace WALLBOX_API
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/
typedef union
{
  uint16_t all;
  struct
  {
    uint16_t current : 8;
    uint16_t phases : 8;
  }data;
}modbusWallbox_currentPhase_t;

typedef union
{
  uint16_t all;
  struct
  {
    uint16_t pass : 1;
    uint16_t failed : 1;
  }flags;
}modbusWallbox_status_t;

typedef union
{
  uint16_t all;
  struct
  {
    uint32_t ac : 1; // bit 0
    uint32_t dc : 1; // bit 1
  }status;
}modbusWallbox_faultCurrent_t;

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/
class ModbusWallbox
{
private:
  bool master = false;
public:

  const uint16_t modbusAddress = 2;

  // static data plug
  MODBUS::ModbusRegister wallboxRegisterMapVer = MODBUS::ModbusRegister(0, master, false);            // plug modbus register map version, must be at address 0
  MODBUS::ModbusRegister hwRevPlug = MODBUS::ModbusRegister(1, master, false);                     // HW revision of the plug PCB
  MODBUS::ModbusRegister hwRevWbPower = MODBUS::ModbusRegister(2, master, false);                     // HW revision of the wallbox power PCB
  MODBUS::ModbusRegister hwRevWbMcu = MODBUS::ModbusRegister(3, master, false);                     // HW revision of the wallbox MCU PCB
  MODBUS::ModbusRegister swRevPlug = MODBUS::ModbusRegister(4, master, false);                     // SW revision of the plug PCB
  MODBUS::ModbusRegister swRevWallbox = MODBUS::ModbusRegister(5, master, false);                     // SW revision of the wallbox PCB
  MODBUS::ModbusRegister statusSelftestPlug = MODBUS::ModbusRegister(6, master, false);           // status of the plug selftest.
  MODBUS::ModbusRegister statusSelftestWallbox = MODBUS::ModbusRegister(7, master, false);         // status of the wallbox selftest.
  MODBUS::ModbusRegister currentPhasesMaxPlug = MODBUS::ModbusRegister(8, master, false);             // max current and number of phases for the plug and cable.
  MODBUS::ModbusRegister currentPhasesMaxWallbox = MODBUS::ModbusRegister(9, master, false);           // max current and number of phases for the wallbox.

  // static data that can be reset, and set by the installer
  MODBUS::ModbusRegister serialNumberLow  = MODBUS::ModbusRegister(20, master, true);                   // Serial number of the wallbox
  MODBUS::ModbusRegister serialNumberHigh = MODBUS::ModbusRegister(21, master, true);                   // Serial number of the wallbox
  MODBUS::ModbusRegister currentMaxInstallation = MODBUS::ModbusRegister(22, master, true);          // max current for installation.
  MODBUS::ModbusRegister phasesInstallation = MODBUS::ModbusRegister(23, master, true);              // number of phases for installation.
  MODBUS::ModbusRegister phasesOrderL1 = MODBUS::ModbusRegister(24, master, true);                   // define which grid phase is connected to the given phase (0 = not connected, 1 = L1, 2 = L2, and 3 = L3)
  MODBUS::ModbusRegister phasesOrderL2 = MODBUS::ModbusRegister(25, master, true);                   // define which grid phase is connected to the given phase (0 = not connected, 1 = L1, 2 = L2, and 3 = L3)
  MODBUS::ModbusRegister phasesOrderL3 = MODBUS::ModbusRegister(26, master, true);                   // define which grid phase is connected to the given phase (0 = not connected, 1 = L1, 2 = L2, and 3 = L3)
  MODBUS::ModbusRegister saveData = MODBUS::ModbusRegister(27, master, true);                   // when changed to 1, register 2x is stored into flash

  // dynamic controlling data
  MODBUS::ModbusRegister evseReserved = MODBUS::ModbusRegister(106, master, true);                  // if no NULL the EVSE is reserved for specified EV's, change color in standby.
  MODBUS::ModbusRegister IL1MaxLow = MODBUS::ModbusRegister(107, master, true);                 // L1 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegister IL1MaxHigh = MODBUS::ModbusRegister(108, master, true);                 // L1 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegister IL2MaxLow = MODBUS::ModbusRegister(109, master, true);                 // L2 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegister IL2MaxHigh = MODBUS::ModbusRegister(110, master, true);                 // L2 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegister IL3MaxLow = MODBUS::ModbusRegister(111, master, true);                // L3 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegister IL3MaxHigh = MODBUS::ModbusRegister(112, master, true);                // L3 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegister IL1Low = MODBUS::ModbusRegister(113, master, false);                   // L1 actual current
  MODBUS::ModbusRegister IL1High = MODBUS::ModbusRegister(114, master, false);                   // L1 actual current
  MODBUS::ModbusRegister IL2Low = MODBUS::ModbusRegister(115, master, false);                   // L2 actual current
  MODBUS::ModbusRegister IL2High = MODBUS::ModbusRegister(116, master, false);                   // L2 actual current
  MODBUS::ModbusRegister IL3Low = MODBUS::ModbusRegister(117, master, false);                   // L3 actual current
  MODBUS::ModbusRegister IL3High = MODBUS::ModbusRegister(118, master, false);                   // L3 actual current
  MODBUS::ModbusRegister UL1Low = MODBUS::ModbusRegister(119, master, false);                   // L1 actual voltage (only measure when power relay is on
  MODBUS::ModbusRegister UL1High = MODBUS::ModbusRegister(120, master, false);                   // L1 actual voltage (only measure when power relay is on
  MODBUS::ModbusRegister UL2Low = MODBUS::ModbusRegister(121, master, false);                   // L2 actual voltage (only measure when power relay is on
  MODBUS::ModbusRegister UL2High = MODBUS::ModbusRegister(122, master, false);                   // L2 actual voltage (only measure when power relay is on
  MODBUS::ModbusRegister UL3Low = MODBUS::ModbusRegister(123, master, false);                   // L3 actual voltage (only measure when power relay is on
  MODBUS::ModbusRegister UL3High = MODBUS::ModbusRegister(124, master, false);                   // L3 actual voltage (only measure when power relay is on
  MODBUS::ModbusRegister powerLow = MODBUS::ModbusRegister(125, master, false);                   // actual power low
  MODBUS::ModbusRegister powerHigh = MODBUS::ModbusRegister(126, master, false);                   // actual power high


  // static charing session data
  MODBUS::ModbusRegister evIdLow = MODBUS::ModbusRegister(150, master, false);                       // ID of the EV, used to authorize charging via OCPP.
  MODBUS::ModbusRegister evIdHigh = MODBUS::ModbusRegister(151, master, false);
  MODBUS::ModbusRegister powerUsedLow = MODBUS::ModbusRegister(152, master, false);                  // number of kwh used in the charging session
  MODBUS::ModbusRegister powerUsedHigh = MODBUS::ModbusRegister(154, master, false);

  // dynamic data wallbox
  MODBUS::ModbusRegister stateLpp = MODBUS::ModbusRegister(200, master, false);                     // state machine light and PP state
  MODBUS::ModbusRegister state61851 = MODBUS::ModbusRegister(201, master, false);                   // state machine iec 61851-1 state
  MODBUS::ModbusRegister socLow = MODBUS::ModbusRegister(202, master, false);                         // state of charge of the EV.
  MODBUS::ModbusRegister socHigh = MODBUS::ModbusRegister(203, master, false);
  MODBUS::ModbusRegister tempInternalPlugLow = MODBUS::ModbusRegister(204, master, false);            // internal temperature (°C of the plug.
  MODBUS::ModbusRegister tempInternalPlugHigh = MODBUS::ModbusRegister(205, master, false);
  MODBUS::ModbusRegister tempInternalWallboxLow = MODBUS::ModbusRegister(206, master, true);    // internal temp of the wallbox.
  MODBUS::ModbusRegister tempInternalWallboxHigh = MODBUS::ModbusRegister(207, master, true);    // internal temp of the wallbox.
  MODBUS::ModbusRegister faultCurrentStatus = MODBUS::ModbusRegister(208, master, true);               // status of ac and dc fault current measuring
  MODBUS::ModbusRegister relayStatus = MODBUS::ModbusRegister(209, master, true);               // status of the ac relay

  /**
   * firmware upload
   */
  MODBUS::ModbusRegister plugNewSwRev = MODBUS::ModbusRegister(1000, master, true);                   // new software version
  MODBUS::ModbusRegister bootActivate = MODBUS::ModbusRegister(1001, master, true);                   // new software activation if = 0x5723, after this is written the plug enter bootloader mode.

  /**
   * special type registers, which uses the normal registers to hold the values.
   */
  MODBUS::ModbusRegisterUint32 serialNumber = MODBUS::ModbusRegisterUint32(&serialNumberLow, &serialNumberHigh);                    // serial number of the device.

  MODBUS::ModbusRegisterFloat soc = MODBUS::ModbusRegisterFloat(&socLow, &socHigh);                    // state of charge of the EV.
  MODBUS::ModbusRegisterUint32 evId = MODBUS::ModbusRegisterUint32(&evIdLow, &evIdHigh);                // ID of the EV, used to authorize charging via OCPP.
  MODBUS::ModbusRegisterUint32 powerUsed = MODBUS::ModbusRegisterUint32(&powerUsedLow, &powerUsedHigh);                // ID of the EV, used to authorize charging via OCPP.

  MODBUS::ModbusRegisterFloat tempInternalPlug = MODBUS::ModbusRegisterFloat(&tempInternalPlugLow, &tempInternalPlugHigh);    // internal temp of the plug.
  MODBUS::ModbusRegisterFloat tempInternalWallbox = MODBUS::ModbusRegisterFloat(&tempInternalWallboxLow, &tempInternalWallboxHigh);    // internal temp of the wallbox.
  MODBUS::ModbusRegisterFloat IL1Max = MODBUS::ModbusRegisterFloat(&IL1MaxLow, &IL1MaxHigh);                 // L1 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegisterFloat IL2Max = MODBUS::ModbusRegisterFloat(&IL2MaxLow, &IL2MaxHigh);                 // L2 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegisterFloat IL3Max = MODBUS::ModbusRegisterFloat(&IL3MaxLow, &IL3MaxHigh);                // L3 OCPP maximum allowed current at the given time
  MODBUS::ModbusRegisterFloat IL1 = MODBUS::ModbusRegisterFloat(&IL1Low, &IL1High);                   // L1 actual current
  MODBUS::ModbusRegisterFloat IL2 = MODBUS::ModbusRegisterFloat(&IL2Low, &IL2High);                   // L2 actual current
  MODBUS::ModbusRegisterFloat IL3 = MODBUS::ModbusRegisterFloat(&IL3Low, &IL3High);                   // L3 actual current
  MODBUS::ModbusRegisterFloat UL1 = MODBUS::ModbusRegisterFloat(&UL1Low, &UL1High);                   // L1 actual voltage (only measure when power relay is on)
  MODBUS::ModbusRegisterFloat UL2 = MODBUS::ModbusRegisterFloat(&UL2Low, &UL2High);                   // L2 actual voltage (only measure when power relay is on)
  MODBUS::ModbusRegisterFloat UL3 = MODBUS::ModbusRegisterFloat(&UL3Low, &UL3High);                   // L3 actual voltage (only measure when power relay is on)
  MODBUS::ModbusRegisterFloat power = MODBUS::ModbusRegisterFloat(&powerLow, &powerHigh);                   // actual power (only measure when power relay is on)

};
/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace WALLBOX_API
#endif



