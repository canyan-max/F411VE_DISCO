/**
 ******************************************************************************
 *@file    :   uart_task.c
 *@brief   :   uartTask — receives ASCII commands over UART2, dispatches to
 *             player_task via player_task_send_cmd().
 *             Frame format: CMD\r\n  (case insensitive)
 *             Supported: play / start / stop / resume
 *@version :   V3.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "uart_task.h"
#include "uart_manager.h"
#include "player_task.h"

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

/* (c | 0x20) converts ASCII uppercase to lowercase */
#define LC(c) ((uint8_t)((c) | 0x20U))

static const uint8_t RSP_PLAY[]    = "ok:play\r\n";
static const uint8_t RSP_START[]   = "ok:start\r\n";
static const uint8_t RSP_STOP[]    = "ok:stop\r\n";
static const uint8_t RSP_RESUME[]  = "ok:resume\r\n";
static const uint8_t RSP_NEXT[]    = "ok:next\r\n";
static const uint8_t RSP_UNKNOWN[] = "err:unknown\r\n";

static void parse_line(const uint8_t *p_data, uint16_t len)
{
    /* strip trailing \r\n */
    while (len > 0U && (p_data[len - 1U] == '\r' || p_data[len - 1U] == '\n'))
    {
        len--;
    }
    if (len == 0U)
    {
        return;
    }

    if (len == 4U &&
        LC(p_data[0]) == 'p' && LC(p_data[1]) == 'l' &&
        LC(p_data[2]) == 'a' && LC(p_data[3]) == 'y')
    {
        player_task_send_cmd(PLAYER_CMD_PLAY);
        uart_manager_send(&s_uart2_mgr, RSP_PLAY, sizeof(RSP_PLAY) - 1U);
    }
    else if (len == 5U &&
             LC(p_data[0]) == 's' && LC(p_data[1]) == 't' &&
             LC(p_data[2]) == 'a' && LC(p_data[3]) == 'r' &&
             LC(p_data[4]) == 't')
    {
        player_task_send_cmd(PLAYER_CMD_START);
        uart_manager_send(&s_uart2_mgr, RSP_START, sizeof(RSP_START) - 1U);
    }
    else if (len == 4U &&
             LC(p_data[0]) == 's' && LC(p_data[1]) == 't' &&
             LC(p_data[2]) == 'o' && LC(p_data[3]) == 'p')
    {
        player_task_send_cmd(PLAYER_CMD_STOP);
        uart_manager_send(&s_uart2_mgr, RSP_STOP, sizeof(RSP_STOP) - 1U);
    }
    else if (len == 6U &&
             LC(p_data[0]) == 'r' && LC(p_data[1]) == 'e' &&
             LC(p_data[2]) == 's' && LC(p_data[3]) == 'u' &&
             LC(p_data[4]) == 'm' && LC(p_data[5]) == 'e')
    {
        player_task_send_cmd(PLAYER_CMD_RESUME);
        uart_manager_send(&s_uart2_mgr, RSP_RESUME, sizeof(RSP_RESUME) - 1U);
    }
    else if (len == 4U &&
             LC(p_data[0]) == 'n' && LC(p_data[1]) == 'e' &&
             LC(p_data[2]) == 'x' && LC(p_data[3]) == 't')
    {
        player_task_send_cmd(PLAYER_CMD_NEXT);
        uart_manager_send(&s_uart2_mgr, RSP_NEXT, sizeof(RSP_NEXT) - 1U);
    }
    else
    {
        uart_manager_send(&s_uart2_mgr, RSP_UNKNOWN, sizeof(RSP_UNKNOWN) - 1U);
#ifdef USER_DEBUG_LOG
        log_w("[uart] unknown cmd len=%u", len);
#endif
    }
}

#undef LC

static void StartUartTask(void *argument)
{
    (void)argument;
    const uint8_t *p_data;
    uint16_t       len;

    for (;;)
    {
        osSemaphoreAcquire(s_sem, osWaitForever);
        while ((p_data = uart_manager_peek(&s_uart2_mgr, &len)) != NULL)
        {
            uint16_t i;
            for (i = 0U; i < len; i++)
            {
                if (p_data[i] == '\n')
                {
                    parse_line(p_data, i + 1U);
                    uart_manager_consume(&s_uart2_mgr, i + 1U);
                    break;
                }
            }
            if (i == len)
            {
                /* no '\n' in this chunk yet, wait for more data */
                break;
            }
        }
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
