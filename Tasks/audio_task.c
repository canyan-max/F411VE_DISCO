/**
 ******************************************************************************
 *@file    :   audio_task.c
 *@brief   :   audioTask — waits for DMA notifications, drives mp3_player.
 *             All RTOS/ISR wiring for audio playback lives here.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "audio_out.h"
#include "mp3_player.h"
#include "audio_task.h"
#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif // end of USER_DEBUG_LOG

/* private function prototypes ----------------------------------------------*/
static void StartAudioTask(void *argument);

/* private variables --------------------------------------------------------*/
static TaskHandle_t audio_handle = NULL;

static const osThreadAttr_t s_attr = {
    .name       = "audioTask",
    .stack_size = 1024 * 24,
    .priority   = (osPriority_t)osPriorityNormal1,
};

/* private functions --------------------------------------------------------*/
static void on_half_cplt(void)
{
    mp3_player_on_half_cplt();
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(audio_handle, &woken);
    portYIELD_FROM_ISR(woken);
}

static void on_cplt(void)
{
    mp3_player_on_cplt();
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(audio_handle, &woken);
    portYIELD_FROM_ISR(woken);
}

static const audio_out_cb_cfg_t s_audio_cb = {
    .pf_half_cplt = on_half_cplt,
    .pf_cplt      = on_cplt,
};

/* exported functions -------------------------------------------------------*/
void audio_task_create(void)
{

    osThreadNew(StartAudioTask, NULL, &s_attr);
}

void audio_task_init(void)
{
    if (audio_out_init(&s_audio_cb) != AUDIO_OUT_OK)
    {
#ifdef USER_DEBUG_LOG
        log_i("audio init err");
#endif // end of USER_DEBUG_LOG

        while(1);
    }
}
static void StartAudioTask(void *argument)
{
    (void)argument;
    portTASK_USES_FLOATING_POINT();
    audio_handle = xTaskGetCurrentTaskHandle();
    for (;;)
    {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0)
        {
            mp3_player_process();
        }
    }
}

void audio_task_signal(void)
{
    if (audio_handle != NULL)
    {
        xTaskNotifyGive(audio_handle);
    }
}

/* end of file --------------------------------------------------------------*/
