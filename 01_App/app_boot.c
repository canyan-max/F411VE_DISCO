/**
 ******************************************************************************
 *@file    :   app_boot.c
 *@brief   :   Application boot orchestrator.
 *             Only file that includes both platform impl headers and task
 *             headers. All ops table wiring happens here.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "app_boot.h"
#include "mcu_uart_hal.h"
#include "decode_task.h"
#include "player_task.h"
#include "uart_task.h"

/* exported functions -------------------------------------------------------*/
void app_boot_init(void)
{
    decode_task_init();
    uart_task_init(&g_uart2_hal_ops);

    decode_task_create();
    player_task_create();
    uart_task_create();
}

/* end of file --------------------------------------------------------------*/
