/**
 ******************************************************************************
 *@file    :   uart_manager.c
 *@brief   :   UART service layer — intermediates between HAL and task.
 *             HAL calls uart_mgr_on_hal_data (manager-owned callback).
 *             Manager then notifies the task via pf_task_notify.
 *             No RTOS dependency. Buffer and DMA fully owned by BSP.
 *@version :   V3.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <stddef.h>
#include "uart_manager.h"
/* private functions --------------------------------------------------------*/

/* HAL calls this (from ISR) — manager owns this callback,task never sees it*/
static void uart_mgr_on_hal_data(void *p_ctx)
{
    uart_manager_t *p_inst = (uart_manager_t *)p_ctx;
    if(p_inst->pf_task_notify != NULL)
    {
        p_inst->pf_task_notify(p_inst->p_task_ctx);
    }
}

/* exported functions -------------------------------------------------------*/
void uart_manager_setup(uart_manager_t      *p_inst,
                        plat_uart_ops_t     *p_ops,
                        uart_mgr_notify_fn_t pf_task_notify,
                        void                *p_task_ctx)
{
    p_inst->p_ops          = p_ops;
    p_inst->pf_task_notify = pf_task_notify;
    p_inst->p_task_ctx     = p_task_ctx;
    p_ops->pf_set_notify(p_inst, uart_mgr_on_hal_data);
}

platform_err_t
uart_manager_send(uart_manager_t *p_inst, const uint8_t *p_buf, uint16_t len)
{
    return p_inst->p_ops->pf_send(p_buf, len);
}

uint16_t uart_manager_available(uart_manager_t *p_inst)
{
    return p_inst->p_ops->pf_available();
}

const uint8_t *uart_manager_peek(uart_manager_t *p_inst, uint16_t *p_len)
{
    return p_inst->p_ops->pf_peek(p_len);
}

void uart_manager_consume(uart_manager_t *p_inst, uint16_t len)
{
    p_inst->p_ops->pf_consume(len);
}

/* end of file --------------------------------------------------------------*/
