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
#include "cs43lxxx_hal.h"    /* g_cs43lxxx_hal_ops, g_cs43l22_drv          */
#include "cs43lxxx_regmap.h" /* CS43L22_REG_* constants for verify reads    */
#include "audio_out.h"
#include "mp3_player.h"
#include <string.h>
#include "elog.h"
#include "fatfs.h"
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
  .name       = "defaultTask",
  .stack_size = 1024 * 4,
  .priority   = (osPriority_t) osPriorityNormal,
};

/* Definitions for audioTask */
osThreadId_t audioTaskHandle;
const osThreadAttr_t audioTask_attributes = {
  .name       = "audioTask",
  .stack_size = 1024 * 24,
  .priority   = (osPriority_t) osPriorityAboveNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static FATFS s_sd_fs;
static FIL   s_sd_file;

static uint32_t sd_src_read(void    *p_ctx,
                            uint32_t offset,
                            uint8_t *p_buf,
                            uint32_t len)
{
    FIL    *p_file = (FIL *)p_ctx;
    UINT    br;
    FRESULT fr;

    if (f_tell(p_file) != (FSIZE_t)offset)
    {
        fr = f_lseek(p_file, (FSIZE_t)offset);
        if (fr != FR_OK)
        {
            return 0U;
        }
    }
    fr = f_read(p_file, p_buf, (UINT)len, &br);
    if (fr != FR_OK)
    {
        return 0U;
    }
    return (uint32_t)br;
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartAudioTask(void *argument);

extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

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
  audioTaskHandle = osThreadNew(StartAudioTask, NULL, &audioTask_attributes);
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
//  MX_USB_HOST_Init();
  /* USER CODE BEGIN StartDefaultTask */
    portTASK_USES_FLOATING_POINT();
    static mp3_src_t s_sd_src;
    FRESULT          fr;

    fr = f_mount(&s_sd_fs, USERPath, 1);
    log_i("[player] f_mount=%d", (int)fr);

    if (fr == FR_OK)
    {
        fr = f_open(&s_sd_file, "0:/SSFF.MP3", FA_READ);
        log_i("[player] f_open=%d size=%lu",
              (int)fr, (unsigned long)f_size(&s_sd_file));

        if (fr == FR_OK)
        {
            s_sd_src.pf_read    = sd_src_read;
            s_sd_src.p_ctx      = &s_sd_file;
            s_sd_src.total_size = (uint32_t)f_size(&s_sd_file);
            mp3_player_start(&s_sd_src);
        }
    }

  /* Infinite loop */
  for(;;)
  {
    /* 播完后等 5s 重播 */
    static uint8_t   s_test_state = 0;
    static uint32_t  s_stop_tick  = 0;

    if (s_test_state == 0 && !mp3_player_is_playing())
    {
        s_stop_tick  = HAL_GetTick();
        s_test_state = 1;
    }
    else if (s_test_state == 1
             && (HAL_GetTick() - s_stop_tick) >= 5000U)
    {
        log_i("replay");
        mp3_player_start(&s_sd_src);
        s_test_state = 0;
    }
    osDelay(5);
  }
  /* USER CODE END StartDefaultTask */
}

void StartAudioTask(void *argument)
{
    portTASK_USES_FLOATING_POINT();
    mp3_player_bind_task(xTaskGetCurrentTaskHandle());
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(portMAX_DELAY));
        mp3_player_process();
    }
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    log_e("STACK OVERFLOW: %s", pcTaskName);
    for (;;);
}
/* USER CODE END Application */

