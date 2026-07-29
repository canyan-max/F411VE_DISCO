/**
 ******************************************************************************
 *@file    :   uart_task.c
 *@brief   :   uartTask — log received bytes for testing (peek/consume, zero-copy).
 *@version :   V2.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "uart_task.h"
#include "uart_manager.h"

#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif

/* private variables --------------------------------------------------------*/
static osSemaphoreId_t s_sem;
static uart_manager_t  s_uart2_mgr;

static const osThreadAttr_t s_attr = {
    .name       = "uartTask",
    .stack_size = 1024U,
    .priority   = (osPriority_t)osPriorityNormal,
};

/* private functions --------------------------------------------------------*/
static void uart2_notify(void *p_ctx)
{
    osSemaphoreRelease((osSemaphoreId_t)p_ctx);
}

static void StartUartTask(void *argument)
{
    (void)argument;
    const uint8_t *p_data;
    uint16_t       len;
    uint32_t       total = 0U;

    for (;;)
    {
        osSemaphoreAcquire(s_sem, osWaitForever);
        while ((p_data = uart_manager_peek(&s_uart2_mgr, &len)) != NULL)
        {
            total += len;
            uart_manager_consume(&s_uart2_mgr, len);
        }
#ifdef USER_DEBUG_LOG
        log_i("[uart] total %u B", total);
#endif
    }
}

/* exported functions -------------------------------------------------------*/
void uart_task_init(plat_uart_ops_t *p_ops)
{
    s_sem = osSemaphoreNew(1U, 0U, NULL);
    uart_manager_setup(&s_uart2_mgr, p_ops, uart2_notify, s_sem);
}

void uart_task_create(void)
{
    osThreadNew(StartUartTask, NULL, &s_attr);
}

/* end of file --------------------------------------------------------------*/
