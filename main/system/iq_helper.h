/*------------------------------------------------------------------------------
* iqHelper.h
* Copyright (C) 2022 IQ-plug - All Rights Reserved
* Created on: 21. okt. 2022
*
* Hold helper macros.
* ----------------------------------------------------------------------------*/
#pragma once
/*------------------------------------------------------------------------------
                        header file
------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
// #include "main.h"
/*------------------------------------------------------------------------------
                        Macro declarations
------------------------------------------------------------------------------*/
// Calculating time in system timer ticks from mS
#define IQ_MS_TO_TICKS(timeMs) ((timeMs * osKernelGetTickFreq()) / 1000u)

// calculate array index count
#define IQ_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// MAximum time in mS
#define IQ_MS_MAX ((HAL_MAX_DELAY / osKernelGetTickFreq()) * 1000u)

#define IQ_ASSERT(arg, fmt, ...) { \
    if (!(arg)) \
    { \
      printf ("\n\r%s @ %d. " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
      while(1); \
    } \
}

#define IQ_ERROR(fmt, ...) { \
      printf ("\n\r*Err - %s @ %d. " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define IQ_INFO(fmt, ...) { \
      printf ("\n\r%d: %s @ %d. " fmt "\n", (int)osKernelGetTickCount(), __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}

#define IQ_MIN(a,b) ((a)<(b)?(a):(b))
#define IQ_MAX(a,b) ((a)>(b)?(a):(b))

/**
  \brief   Reverse byte order (16 bit)
  \details Reverses the byte order in a 16-bit value and returns the signed 16-bit result. For example, 0x0080 becomes 0x8000.
  \param [in]    value  Value to reverse
  \return               Reversed value
 */
static inline int16_t __REVSH(int16_t value)
{
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
  return (int16_t)__builtin_bswap16(value);
#else
  int16_t result;

  __ASM volatile ("revsh %0, %1" : __CMSIS_GCC_OUT_REG (result) : __CMSIS_GCC_USE_REG (value) );
  return result;
#endif
}
//
///**
//  \brief   Reverse byte order (32 bit)
//  \details Reverses the byte order in unsigned integer value. For example, 0x12345678 becomes 0x78563412.
//  \param [in]    value  Value to reverse
//  \return               Reversed value
// */
//static inline uint32_t __REV(uint32_t value)
//{
//#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
//  return __builtin_bswap32(value);
//#else
//  uint32_t result;
//
//  __ASM volatile ("rev %0, %1" : __CMSIS_GCC_OUT_REG (result) : __CMSIS_GCC_USE_REG (value) );
//  return result;
//#endif
//}

/**
  \brief   Reverse byte order by using memcpy. Can be used to floats.
  \details Reverses the byte order. For example, 0x12345678 becomes 0x78563412.
  \param [in]    pValue pointer to the value to reverse
  \param [in]    size   size of the value
 */
static inline void __REVMULTI(void* pValue, size_t size)
{
  size_t index;
  uint8_t* dataArray = (uint8_t*)pValue;
  for (index = 0; index < size/2; index++)
  {
    uint8_t temp = dataArray[index];
    dataArray[index] = dataArray[size - index];
    dataArray[size - index] = temp;
  }
}



#ifdef __cplusplus
extern "C"
namespace MY_NAMESPACE
{
/*------------------------------------------------------------------------------
                        Type declarations
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        classes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
                        Freestanding function declarations
------------------------------------------------------------------------------*/
#endif



#ifdef __cplusplus
}  // namespace MY_NAMESPACE
#endif



