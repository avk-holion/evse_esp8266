/*------------------------------------------------------------------------------
* observer.cpp
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 3. feb. 2023
*
* Fill in a description
* ----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <cmsis_os2.h>
#include <string.h>

#include "iq_helper.h"
#include "observer.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/

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
namespace SYSTEM
{
Observer::Observer(ObserverCb_t list[], uint32_t count)
{
  this->listCb = list;
  memset(listCb, 0, (sizeof(listCb[0]) * count));
  this->listCount = count;
}

bool Observer::add(ObserverCb_t callback)
{
  bool success = true;
  int32_t index = findIndex(callback);

  if (index < 0)
  {
    // not set already, find next free index
    index = findIndex(NULL);
    if (index < 0)
    {
      success = false;
    }
    else
    {
      listCb[index] = callback;
    }
  }

  return success;

}

void Observer::remove(ObserverCb_t callback)
{
  int32_t index = findIndex(callback);

  if (index >= 0)
  {
    listCb[index] = NULL;
  }
}

void Observer::notify (void* argPtr)
{
  uint32_t index = 0;

  for (index = 0; index < listCount; index++)
  {
    if (listCb[index] != NULL)
    {
      listCb[index](argPtr);
    }
  }
}

int32_t Observer::findIndex(ObserverCb_t pointer)
{
  int32_t index = -1;

  size_t n;
  for(n = 0; n < listCount; n++)
  {
    if (pointer == listCb[n])
    {
      index = n;
      break;
    }
  }

  return index;
}

}  // namespace end SYSTEM
