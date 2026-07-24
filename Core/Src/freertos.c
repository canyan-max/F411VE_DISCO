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
#include "media_src.h"
#include "mp3_player.h"
#include "sd_manager.h"
#include "audio_task.h"
#include "usart.h"
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
osThreadId_t         defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name       = "defaultTask",
    .stack_size = 1024 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_USB_HOST_Init(void);
void        MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 2 */
#ifdef USER_DEBUG_LOG
char pcWriteBuffer[512];
#endif
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
#ifdef USER_DEBUG_LOG
    static uint32_t ulIdleCount = 0;
    ulIdleCount++;

    if((ulIdleCount % 10000000) == 0)
    {
        vTaskList(pcWriteBuffer);
        log_i("Task Info:\n%s\n", pcWriteBuffer);
    }
#endif // USER_DEBUG_LOG
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
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
void MX_FREERTOS_Init(void)
{
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
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
                                    &defaultTask_attributes);

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
     MX_USB_HOST_Init();
     /* USER CODE BEGIN StartDefaultTask */
     static media_src_t s_src;
     (void)argument;
     if(sd_manager_mount() == SD_OK)
     {
 #ifdef USER_DEBUG_LOG
         log_i("[sd] mount ok");
 #endif // USER_DEBUG_LOG
         if(sd_manager_open("0:/SSFF.MP3") == SD_OK)
         {
             sd_manager_get_src(&s_src);
             mp3_player_start(&s_src);
             audio_task_signal();
         }
         else
         {
 #ifdef USER_DEBUG_LOG
             log_e("[sd] open failed");
 #endif // USER_DEBUG_LOG
         }
     }
     else
     {
 #ifdef USER_DEBUG_LOG
         log_e("[sd] mount failed");
 #endif // USER_DEBUG_LOG
     }
     /* Infinite loop */
     for(;;)

     {
         static uint8_t s_test_state = 0;
         static uint32_t s_stop_tick = 0;
         if(s_test_state == 0 && !mp3_player_is_playing())
         {
             s_stop_tick = HAL_GetTick();
             s_test_state = 1;
         }
         else if(s_test_state == 1
                 && (HAL_GetTick() - s_stop_tick) >= 5000U)
        
         {
 #ifdef USER_DEBUG_LOG
         log_i("replay");
 #endif // end of USER_DEBUG_LOG
             sd_manager_close();
             if(sd_manager_open("0:/SSFF.MP3") == SD_OK)
             {
                 sd_manager_get_src(&s_src);
                 mp3_player_start(&s_src);
                 audio_task_signal();
             }
         }    
         osDelay(1000);
     }
     /* USER CODE END StartDefaultTask */
 }
/* USER CODE END Application */
