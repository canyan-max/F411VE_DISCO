/**
 ******************************************************************************
 *@file    :   service_uart_manager.h
 *@brief   :   UART service layer — owns the HAL binding and task notification.
 *             HAL calls manager's internal callback; manager notifies the task.
 *             Task registers with manager only; task never sees uart_hal types.
 *@version :   V3.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef SERVICE_UART_MANAGER_H
#define SERVICE_UART_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include "bsp_uart.h"

/* typedef ------------------------------------------------------------------*/

/* Task-facing notification: manager calls this when new data arrives.       */
/* Signature matches uart_notify_fn_t intentionally — same calling convention*/
/* but represents a different boundary (manager → task, not HAL → task).    */
typedef void (*uart_mgr_notify_fn_t)(void *p_ctx);

typedef struct
{
    plat_uart_ops_t     *p_ops;
    uart_mgr_notify_fn_t pf_task_notify;
    void                *p_task_ctx;
} uart_manager_t;

/* functions ----------------------------------------------------------------*/
/**
 * @brief  Bind manager to a BSP ops table, register task notification, start RX.
 *         Manager registers its own internal callback with HAL — task never
 *         touches uart_notify_fn_t directly.
 */
void           uart_manager_setup(uart_manager_t      *p_inst,
                                   plat_uart_ops_t     *p_ops,
                                   uart_mgr_notify_fn_t pf_task_notify,
                                   void                *p_task_ctx);

platform_err_t uart_manager_send(uart_manager_t *p_inst,
                                  const uint8_t  *p_buf,
                                  uint16_t        len);

uint16_t       uart_manager_available(uart_manager_t *p_inst);

/**
 * @brief  Zero-copy read: returns pointer into BSP's DMA buffer + length.
 *         Returns NULL when empty. Caller MUST call uart_manager_consume().
 */
const uint8_t *uart_manager_peek(uart_manager_t *p_inst, uint16_t *p_len);

void           uart_manager_consume(uart_manager_t *p_inst, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_UART_MANAGER_H */
