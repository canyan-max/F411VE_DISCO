/**
 ******************************************************************************
 *@file    :   uart_hal.c
 *@brief   :   STM32 USART2 BSP — Circular DMA into own kfifo, self-contained.
 *             TX: blocking (HAL_UART_Transmit)
 *             RX: Circular DMA + IDLE interrupt; ISR reads NDTR directly;
 *                 upper layer reads via pf_peek / pf_consume (zero-copy).
 *@version :   V2.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "uart_hal.h"
#include "usart.h"
#include "kfifo.h"

/* private define -----------------------------------------------------------*/
#define UART2_DMA_BUF_SIZE (1024U)

/* private variables --------------------------------------------------------*/
static uint8_t          s_dma_buf[UART2_DMA_BUF_SIZE];
static kfifo_t          s_kfifo;
static uint16_t         s_prev_pos   ;
static void            *s_notify_ctx ;
static uart_notify_fn_t s_notify     ;

/* private functions --------------------------------------------------------*/
static uart_status_t uart2_send(const uint8_t *p_buf, uint16_t len)
{
    HAL_StatusTypeDef ret = HAL_UART_Transmit(&huart2, (uint8_t *)p_buf,
                                               len, HAL_MAX_DELAY);
    return (ret == HAL_OK) ? UART_OK : UART_ERR;
}

static void uart2_set_notify(void *p_ctx, uart_notify_fn_t pf_notify)
{
    s_notify_ctx = p_ctx;
    s_notify     = pf_notify;
    kfifo_init(&s_kfifo, s_dma_buf, UART2_DMA_BUF_SIZE);
    s_prev_pos = 0U;
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, s_dma_buf, UART2_DMA_BUF_SIZE);
}

static uint16_t uart2_available(void)
{
    return (uint16_t)kfifo_len(&s_kfifo);
}

static const uint8_t *uart2_peek(uint16_t *p_len)
{
    uint32_t avail   = kfifo_len(&s_kfifo);
    uint32_t contig  = kfifo_contig_read_data(&s_kfifo);
    uint32_t readable = (avail < contig) ? avail : contig;
    if (readable == 0U || p_len == NULL)
    {
        if (p_len != NULL)
        {
            *p_len = 0U;
        }
        return NULL;
    }
    *p_len = (uint16_t)readable;
    return s_kfifo.buffer + (s_kfifo.out & (s_kfifo.size - 1U));
}

static void uart2_consume(uint16_t len)
{
    kfifo_advance_out(&s_kfifo, (uint32_t)len);
}

/* exported variables -------------------------------------------------------*/
uart_hal_ops_t g_uart2_hal_ops = {
    .pf_send       = uart2_send,
    .pf_set_notify = uart2_set_notify,
    .pf_available  = uart2_available,
    .pf_peek       = uart2_peek,
    .pf_consume    = uart2_consume,
};

/* callbacks ----------------------------------------------------------------*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart == &huart2)
    {
        /* size: IDLE → current pos, HT → buf/2, TC(Circular) → buf_size.  */
        /* All three work correctly with the delta formula below.           */
        uint16_t delta = (uint16_t)((size - s_prev_pos) &
                                     (UART2_DMA_BUF_SIZE - 1U));
        if (delta > 0U)
        {
            kfifo_advance_in(&s_kfifo, (uint32_t)delta);
            s_prev_pos = size & (UART2_DMA_BUF_SIZE - 1U);
            if (s_notify != NULL)
            {
                s_notify(s_notify_ctx);
            }
        }
    }
}

/* end of file --------------------------------------------------------------*/
