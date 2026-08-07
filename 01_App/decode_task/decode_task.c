/**
 ******************************************************************************
 *@file    :   decode_task.c
 *@brief   :   decodeTask — waits for DMA half/cplt notifications, drives
 *             mp3_player decode loop. No SD or playback control here.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "device_audio_out.h"
#include "service_mp3_player.h"
#include "decode_task.h"
#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif

/* private function prototypes ----------------------------------------------*/
static void StartDecodeTask(void *argument);

/* private variables --------------------------------------------------------*/
static TaskHandle_t s_decode_handle = NULL;

static const osThreadAttr_t s_attr = {
    .name       = "decodeTask",
    .stack_size = 1024U * 24U,
    .priority   = (osPriority_t)osPriorityNormal1,
};

static void on_half_cplt(void)
{
    mp3_player_on_half_cplt();
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(s_decode_handle, &woken);
    portYIELD_FROM_ISR(woken);
}

static void on_cplt(void)
{
    mp3_player_on_cplt();
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(s_decode_handle, &woken);
    portYIELD_FROM_ISR(woken);
}

static const audio_out_cb_cfg_t s_audio_cb = {
    .pf_half_cplt = on_half_cplt,
    .pf_cplt      = on_cplt,
};

/* exported functions -------------------------------------------------------*/
void decode_task_init(void)
{
    if (audio_out_init(&s_audio_cb) != PLATFORM_ERR_OK)
    {
#ifdef USER_DEBUG_LOG
        log_e("audio_out init failed");
#endif
        while (1);
    }
}

void decode_task_create(void)
{
    osThreadNew(StartDecodeTask, NULL, &s_attr);
}

void decode_task_signal(void)
{
    if (s_decode_handle != NULL)
    {
        xTaskNotifyGive(s_decode_handle);
    }
}

static void StartDecodeTask(void *argument)
{
    (void)argument;
    portTASK_USES_FLOATING_POINT();
    s_decode_handle = xTaskGetCurrentTaskHandle();
    for (;;)
    {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0)
        {
            mp3_player_process();
        }
    }
}

/* end of file --------------------------------------------------------------*/
