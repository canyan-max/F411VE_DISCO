/**
 ******************************************************************************
 *@file    :   bsp_uart.h
 *@brief   :   UART BSP interface contract.
 *             Defines the ops table that any MCU's UART platform layer must
 *             implement. Types here never change when switching MCU — only the
 *             implementation in Platform/ changes.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef BSP_UART_H
#define BSP_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include <stdint.h>

/* typedef ------------------------------------------------------------------*/
typedef enum
{
    UART_OK = 0,
    UART_ERR_BUSY,
    UART_ERR_TIMEOUT,
    UART_ERR,
} uart_status_t;

/* One-way upward notification — no return value, platform never waits on    */
/* upper layer response.                                                      */
typedef void (*uart_notify_fn_t)(void *p_ctx);

/* Ops table: the contract any MCU platform layer must fulfill.              */
/* Switch MCU = rewrite the implementor; this struct stays the same.         */
typedef struct
{
    uart_status_t  (*pf_send)(const uint8_t *p_buf, uint16_t len);
    void           (*pf_set_notify)(void             *p_ctx,
                                    uart_notify_fn_t  pf_notify);
    uint16_t       (*pf_available)(void);
    const uint8_t *(*pf_peek)(uint16_t *p_len);
    void           (*pf_consume)(uint16_t len);
} uart_hal_ops_t;

#ifdef __cplusplus
}
#endif

#endif /* BSP_UART_H */
