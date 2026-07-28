/**
 ******************************************************************************
 *@file    :   uart_hal.h
 *@brief   :   STM32F4 UART platform implementation — instance declaration.
 *             Types are defined in bsp_uart.h (stable interface contract).
 *             Switch MCU: replace uart_hal.c + this file; bsp_uart.h unchanged.
 *@version :   V2.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef UART_HAL_H
#define UART_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include "bsp_uart.h"

/* variables ----------------------------------------------------------------*/
extern uart_hal_ops_t g_uart2_hal_ops;

#ifdef __cplusplus
}
#endif

#endif /* UART_HAL_H */
