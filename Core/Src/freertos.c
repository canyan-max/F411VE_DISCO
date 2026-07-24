/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "media_src.h"
#include "mp3_player.h"
#include "sd_manager.h"
#include "fatfs.h"
#include "audio_task.h"
#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif // USER_DEBUG_LOG
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 2 */

#if defined(USER_IDLE_LOG) && defined(USER_DEBUG_LOG)
char pcWriteBuffer[512];
static uint32_t ulIdleCount = 0;
#endif //end of defined(USER_IDLE_LOG) && defined(USER_DEBUG_LOG)

void vApplicationIdleHook(void)
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
    task. It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()). If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
#if defined(USER_IDLE_LOG) && defined(USER_DEBUG_LOG)
    ulIdleCount++;

    if ((ulIdleCount % 10000000) == 0)
    {
        vTaskList(pcWriteBuffer);
        log_i("Task Info:\n%s\n", pcWriteBuffer);
    }
#endif // end of defined(USER_IDLE_LOG) && defined(USER_DEBUG_LOG)
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
static uint8_t find_next_mp3(const FILINFO *p_files, uint8_t count,
                              uint8_t start)
{
    if (count == 0)
    {
        return 0xFF;
    }
    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t     idx = (uint8_t)((start + i) % count);
        size_t      len = strlen(p_files[idx].fname);
        if (len < 4)
        {
            continue;
        }
        const char *ext = p_files[idx].fname + len - 4;
        if (ext[0] == '.' &&
            (ext[1] == 'M' || ext[1] == 'm') &&
            (ext[2] == 'P' || ext[2] == 'p') &&
            ext[3] == '3')
        {
            return idx;
        }
    }
    return 0xFF;
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
    called if a stack overflow is detected. */
    configASSERT((pcTaskName != NULL) && (xTask != NULL));
#ifdef USER_DEBUG_LOG
    log_e("STACK OVERFLOW Task: %s (TCB: 0x%p)", pcTaskName, xTask);
#endif // USER_DEBUG_LOG

#if (configUSE_TRACE_FACILITY == 1)
    {
#ifdef USER_DEBUG_LOG
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(xTask);
        log_e("Remaining Stack (min): %d words", uxHighWaterMark);
#endif // USER_DEBUG_LOG
    }

#endif
    for(;;)
    {
    }
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    audio_task_init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    audio_task_create();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_HOST */
//   MX_USB_HOST_Init();
  /* USER CODE BEGIN StartDefaultTask */
    static media_src_t s_src;
    static FILINFO     s_files[16];
    static char        s_path[32];
    static char        s_log_buf[64];
    static uint32_t    s_play_count = 0;
    static uint16_t    s_song_counts[16];
    uint8_t s_file_count = 0;
    uint8_t s_play_idx   = 0xFF;
    (void)argument;

    if (sd_manager_mount(USERPath) != SD_OK)
    {
#ifdef USER_DEBUG_LOG
        log_e("[sd] mount failed");
#endif
        for (;;)
        {
            osDelay(1000);
        }
    }

//    sd_manager_list_dir("0:/", s_files, 16, &s_file_count);
    sd_manager_list_dir(USERPath, s_files, 16, &s_file_count);
#ifdef USER_DEBUG_LOG
    log_i("[sd] mount ok, %d files", s_file_count);
    for (uint8_t i = 0; i < s_file_count; i++)
    {
        log_i("[sd] %s  %lu B", s_files[i].fname, s_files[i].fsize);
    }
#endif

    s_play_idx = find_next_mp3(s_files, s_file_count, 0);
    if (s_play_idx != 0xFF)
    {
        snprintf(s_path, sizeof(s_path), "0:/%s", s_files[s_play_idx].fname);
        if (sd_manager_open(s_path) == SD_OK)
        {
            sd_manager_get_src(&s_src);
            mp3_player_start(&s_src);
            audio_task_signal();
#ifdef USER_DEBUG_LOG
            log_i("[sd] playing %s", s_files[s_play_idx].fname);
#endif // end of USER_DEBUG_LOG
        }
    }

    /* Infinite loop */
    for (;;)
    {
        if (s_play_idx != 0xFF && !mp3_player_is_playing())
        {
            sd_manager_close();
            s_play_count++;
            s_song_counts[s_play_idx]++;
            int log_len = snprintf(s_log_buf, sizeof(s_log_buf),
                                   "[%u] %s %lu B (x%u)\r\n",
                                   s_play_count,
                                   s_files[s_play_idx].fname,
                                   s_files[s_play_idx].fsize,
                                   s_song_counts[s_play_idx]);
            if (log_len > 0 && sd_manager_open_write("0:/mp3log.txt") == SD_OK)
            {
                sd_manager_write((const uint8_t *)s_log_buf,
                                 (uint32_t)log_len, NULL);
                sd_manager_close_write();
            }
            uint8_t next = find_next_mp3(s_files, s_file_count,
                                         (uint8_t)((s_play_idx + 1U) %
                                                    s_file_count));
            if (next != 0xFF)
            {
                s_play_idx = next;
                snprintf(s_path, sizeof(s_path), "0:/%s",
                         s_files[s_play_idx].fname);
                if (sd_manager_open(s_path) == SD_OK)
                {
                    sd_manager_get_src(&s_src);
                    mp3_player_start(&s_src);
                    audio_task_signal();
#ifdef USER_DEBUG_LOG
                    log_i("[sd] playing %s", s_files[s_play_idx].fname);
#endif // end of USER_DEBUG_LOG
                }
            }
        }
        osDelay(500);
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

