/**
*********************************************************************************
*@file               :   bsp_dwt.c
*
*@brief              :   Provide the HAL APIs of dwt              
                         The content of this source file has been tested.
                         The coterx-M7 core must write to the register LAR 
                         0xC5ACCE55
*@version            :   V1.0 
*@note               :   1 tab == 4 spaces!
*********************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "bsp_dwt.h"
//#include "stm32h7xx.h"
#include "stm32f4xx.h"
#include "core_cm4.h"
/* Define   ------------------------------------------------------------------*/
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
  #include <stdint.h>
  extern uint32_t SystemCoreClock;
#endif
/* Private  variables --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private  functions  -------------------------------------------------------*/

//static uint32_t get_cpu_freq(void)
//{
//    return 
//HAL_RCC_GetSysClockFreq() >> D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos];
//}
/* Exported functions --------------------------------------------------------*/
/** 
* @brief     	 	:  dwt_init
* @retval      		:  None
* @param      	 	:  None 
*/
void dwt_init(void)
{
    // set the demcr bit24  
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // enable write access only m7
#ifdef STM32H7
    DWT->LAR = 0xC5ACCE55;
#endif   // end of  STM32H7
    //clear dwt tick 
    DWT->CYCCNT =0;
    // enable the cyccnt reg 
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/** 
* @brief     	 	:  dwt_delay_us
* @retval      		:  None
* @param      	 	:  delay times for us
*/
void dwt_delay_us(uint32_t us)
{
    // check if the input parameters are valid
    if (0 == us) return;
    // calculate the us clock using the system clock
    uint32_t ticks_per_us = SystemCoreClock / 1000000;
    if (ticks_per_us == 0) ticks_per_us = 1;
    uint32_t delay_ticks = us * ticks_per_us;
    uint32_t start_tick = DWT->CYCCNT;
    uint32_t elapsed_ticks;
    // waiting the delay time finished
    do 
    {
        uint32_t current_tick = DWT->CYCCNT;
        if (current_tick >= start_tick) 
        {
            elapsed_ticks = current_tick - start_tick;
        } 
        else 
        {
            elapsed_ticks = (UINT32_MAX - start_tick) + \
            current_tick + 1;
        }
    } while (elapsed_ticks < delay_ticks);
}

/** 
* @brief     	 	:  dwt_ticks_to_us
* @retval      		:  None
* @param      	 	:  ticks dwt ticks 
*/
static uint32_t dwt_ticks_to_us(uint32_t ticks)
{
    // 64-bit data must be used to prevent data overflow.
    uint64_t time_us = (uint64_t)ticks * 1000000ULL / SystemCoreClock;
    return (uint32_t)time_us;
}

/** 
* @brief     	 	:  get_dwt_us
* @retval      		:  us
* @param      	 	:  none
*/
uint32_t get_dwt_us(void)
{
    return dwt_ticks_to_us(DWT->CYCCNT);
}



