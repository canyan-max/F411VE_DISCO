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

void uart_task_init(void);
void uart_task_create(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_TASK_H */
