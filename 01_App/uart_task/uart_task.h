/**
 ******************************************************************************
 *@file    :   uart_task.h
 *@brief   :   uartTask — UART receive/send scheduling task.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef UART_TASK_H
#define UART_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_uart.h"

void uart_task_init(plat_uart_ops_t *p_ops);
void uart_task_create(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_TASK_H */
