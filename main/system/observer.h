/*------------------------------------------------------------------------------
* observer.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 3. feb. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>

/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
namespace SYSTEM
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/
class Observer
{
public:
  typedef void (*ObserverCb_t)(void* arg);
  Observer(ObserverCb_t list[], uint32_t count);
  bool add(ObserverCb_t callback);
  void remove(ObserverCb_t callback);
  void notify(void* argPtr);
private:
  ObserverCb_t* listCb;
  uint32_t listCount;

  int32_t findIndex(ObserverCb_t pointer);
};
/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif


#ifdef __cplusplus
}  // namespace SYSTEM
#endif



