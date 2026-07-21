/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "elog.h"
#include "cs43lxxx_hal.h"    /* g_cs43lxxx_hal_ops, g_cs43l22_drv           */
#include "cs43lxxx_regmap.h" /* CS43L22_REG_* constants for verify reads    */
#include "audio_out.h"
#include "mp3_player.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void elog_init_handler(void);


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
    elog_init_handler();

    mp3_player_init();
    {
        uint8_t           rd_val = 0;
        cs43lxxx_status_t rd_ret = CS43LXXX_STATUS_OK;

        /* Chip ID */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_ID, &rd_val, 1);
        log_i("[verify] ID(0x%02X) ret=%d val=0x%02X mask=0x%02X",
              CS43L22_REG_ID, rd_ret, rd_val, rd_val & CS43L22_CHIP_ID_MASK);

        /* POWER_CTL1: expected 0x01 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_POWER_CTL1,
                 &rd_val, 1);
        log_i("[verify] POWER_CTL1(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_POWER_CTL1, rd_ret, rd_val, 0x01);

        /* CLOCKING_CTL: expected 0x81 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_CLOCKING_CTL,
                 &rd_val, 1);
        log_i("[verify] CLOCKING_CTL(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_CLOCKING_CTL, rd_ret, rd_val, 0x81);

        /* INTERFACE_CTL1: expected 0x04 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_INTERFACE_CTL1,
                 &rd_val, 1);
        log_i("[verify] INTERFACE_CTL1(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_INTERFACE_CTL1, rd_ret, rd_val, 0x04);

        /* MISC_CTL: expected 0x04 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_MISC_CTL,
                 &rd_val, 1);
        log_i("[verify] MISC_CTL(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_MISC_CTL, rd_ret, rd_val, 0x04);

        /* TONE_CTL: expected 0x0F */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_TONE_CTL,
                 &rd_val, 1);
        log_i("[verify] TONE_CTL(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_TONE_CTL, rd_ret, rd_val, 0x0F);

        /* INTERFACE_CTL2: expected 0x00 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_INTERFACE_CTL2,
                 &rd_val, 1);
        log_i("[verify] INTERFACE_CTL2(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_INTERFACE_CTL2, rd_ret, rd_val, 0x00);

        /* PASSTHR_A_SELECT: expected 0x00 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_PASSTHR_A_SELECT,
                 &rd_val, 1);
        log_i("[verify] PASSTHR_A(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_PASSTHR_A_SELECT, rd_ret, rd_val, 0x00);

        /* PLAYBACK_CTL1: expected 0x00 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_PLAYBACK_CTL1,
                 &rd_val, 1);
        log_i("[verify] PLAYBACK_CTL1(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_PLAYBACK_CTL1, rd_ret, rd_val, 0x00);

        /* PCMA_VOL: expected 0x00 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_PCMA_VOL,
                 &rd_val, 1);
        log_i("[verify] PCMA_VOL(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_PCMA_VOL, rd_ret, rd_val, 0x00);

        /* CHARGE_PUMP_FREQ: expected 0x05 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_CHARGE_PUMP_FREQ,
                 &rd_val, 1);
        log_i("[verify] CHG_PUMP(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_CHARGE_PUMP_FREQ, rd_ret, rd_val, 0x05);

        /* POWER_CTL2: expected 0x05 */
        rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                 CS43L22_REG_POWER_CTL2,
                 &rd_val, 1);
        log_i("[verify] POWER_CTL2(0x%02X) ret=%d val=0x%02X expect=0x%02X",
              CS43L22_REG_POWER_CTL2, rd_ret, rd_val, 0x05);

    }

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while(1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 254;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 5;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void elog_init_handler(void)
{
    elog_init();
    elog_set_text_color_enabled(true);
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_DEBUG,
                 ELOG_FMT_ALL &
                 ~(ELOG_FMT_P_INFO | ELOG_FMT_T_INFO | ELOG_FMT_TIME));
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);
    elog_start();
}

/* I2S DMA TX complete callback */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if(hi2s == &hi2s3)
    {
        audio_out_tx_cplt();
//        log_i("[i2s] TxfinshCallback");
    }
}

/* I2S DMA TX half-complete callback */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if(hi2s == &hi2s3)
    {
        audio_out_tx_half_cplt();
//        log_i("[i2s] TxHalfCallback");
    }
}

/* I2S error callback */
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if(hi2s == &hi2s3)
    {
        uint32_t err = HAL_I2S_GetError(hi2s);
        // log_i("[i2s] ErrorCallback: err=0x%08lX", err);
    }
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM10 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM10)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq();

    while(1)
    {
    }

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
