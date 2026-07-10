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
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_ONLY_MP3     1
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#define MAIN_TAG      "main"
#include "elog.h"
#include "bsp_cs43lxxx_drv.h" /* bsp_cs43lxxx_drv lib header file. */
#include "cs43lxxx_regmap.h" /* cs43lxxx_regmap lib header file. */
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
static volatile uint8_t dma_tx_done = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
extern cs43lxxx_hal_ops_t g_cs43lxxx_hal_ops;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 
 * @brief  Initialize the elog library.
 */
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
  MX_USB_HOST_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  elog_init_handler();

  /* Reconfigure I2S3 GPIOs: PC7(MCK), PC10(SCK), PC12(SD)
   * 1) Force MODER to AF — raw write was unreliable for PC12
   * 2) Bump speed to HIGH — MCK=24.576MHz, SCK=6.144MHz, LOW(~2MHz) is insufficient
   */
  /* Power-on reset: Audio_RST low → delay → high → delay */


  /* Test cs43lxxx_instruct */
  /* Step 2: 1kHz sine @ 96kHz SR → 96 samples/cycle, pre-computed LUT.
*         Amplitude ~50% of full-scale (16384 / 32767). */
static const int16_t sine_1khz[96] = {
      0,  1072,  2139,  3196,  4240,  5266,  6270,  7247,
  8192,  9102,  9972, 10799, 11579, 12309, 12985, 13604,
  14163, 14659, 15090, 15454, 15749, 15973, 16126, 16206,
  16213, 16147, 16008, 15797, 15515, 15164, 14744, 14259,
  13711, 13102, 12436, 11716, 10946, 10130,  9271,  8374,
  7444,  6484,  5499,  4494,  3473,  2441,  1403,   363,
  -673, -1701, -2716, -3713, -4688, -5636, -6553, -7435,
  -8278, -9077, -9829,-10531,-11179,-11770,-12300,-12767,
-13168,-13501,-13763,-13954,-14071,-14115,-14085,-13982,
-13805,-13557,-13239,-12852,-12400,-11885,-11310,-10680,
  -9997, -9266, -8492, -7678, -6828, -5948, -5042, -4114,
  -3170, -2214, -1251,  -286
};
  cs43xxx_drv_t      cs43l22_drv   = {0};
  cs43lxxx_status_t instruct_ret = cs43lxxx_instruct(&cs43l22_drv,
                                                      &g_cs43lxxx_hal_ops,
                                                      CS43XXX_I2C_ADDR_7BIT,
                                                      OUTPUT_DEVICE_AUTO);
  log_i("instruct ret=%d, is_init=%d", instruct_ret, cs43l22_drv.is_init);

  /* Verify: read back key registers to confirm writes took effect. */
  if(CS43LXXX_STATUS_OK == instruct_ret)
  {
      uint8_t           rd_val  = 0;
      cs43lxxx_status_t rd_ret  = CS43LXXX_STATUS_OK;

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
            CS43L22_REG_POWER_CTL2, rd_ret, rd_val, 0xAA);

      /* ---- I2S playback test ---- */
      /* Step 1: Activate playback path */
      cs43lxxx_status_t play_ret = cs43l22_drv.pf_play(&cs43l22_drv);
      log_i("[i2s] play ret=%d", play_ret);

      /* Verify play registers */
      rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                                                    CS43L22_REG_POWER_CTL1,
                                                    &rd_val, 1);
      log_i("[i2s] POWER_CTL1(0x%02X) ret=%d val=0x%02X expect=0x%02X",
            CS43L22_REG_POWER_CTL1, rd_ret, rd_val, 0x9E);

      rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                                                    CS43L22_REG_MISC_CTL,
                                                    &rd_val, 1);
      log_i("[i2s] MISC_CTL(0x%02X) ret=%d val=0x%02X expect=0x%02X",
            CS43L22_REG_MISC_CTL, rd_ret, rd_val, 0x06);

      rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                                                    CS43L22_REG_POWER_CTL2,
                                                    &rd_val, 1);
      log_i("[i2s] POWER_CTL2(0x%02X) ret=%d val=0x%02X expect=0x%02X",
            CS43L22_REG_POWER_CTL2, rd_ret, rd_val, 0xAA);



      /* Step 3: Switch DMA to CIRCULAR mode so the tone plays continuously.
       *         NORMAL mode stops after 96 samples (1 ms) — inaudible click. */
      // hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
      // HAL_DMA_Init(&hdma_spi3_tx);

      /* Step 4: Start I2S circular DMA — plays forever until stopped */
      cs43lxxx_status_t i2s_ret = g_cs43lxxx_hal_ops.pf_i2s_transmit_with_dma(
                                      (uint16_t *)sine_1khz, 96);
      dma_tx_done=0;                                      
      log_i("[i2s] dma circular start ret=%d (size=96, ~%d Hz)",
            i2s_ret, 96000 / 96);

      /* Step 5: DEBUG — wait for a few DMA cycles then check clocks DURING playback */
      g_cs43lxxx_hal_ops.pf_delay_ms(200);

      /* CLK_STATUS during active DMA (circular → clocks are running) */
      rd_ret = g_cs43lxxx_hal_ops.pf_i2c_read_reg(CS43XXX_I2C_ADDR_7BIT,
                                                    CS43L22_REG_OVF_CLK_STATUS,
                                                    &rd_val, 1);
      log_i("[i2s] CLK_STATUS(0x%02X) ret=%d val=0x%02X (bit6=PLL_LCK,bit4=SCLK,bit3=LRCK)",
            CS43L22_REG_OVF_CLK_STATUS, rd_ret, rd_val);

      /* STM32 I2S3 registers */
      log_i("[i2s] I2S3 CR1=0x%08lX CR2=0x%08lX SR=0x%08lX",
            hi2s3.Instance->I2SCFGR, hi2s3.Instance->I2SPR, hi2s3.Instance->SR);

      /* RCC + GPIO diagnostics */
      log_i("[dbg] APB1ENR=0x%08lX PLLI2SCFGR=0x%08lX",
            RCC->APB1ENR, RCC->PLLI2SCFGR);
      log_i("[dbg] GPIOA_MODER=0x%08lX GPIOC_MODER=0x%08lX",
            GPIOA->MODER, GPIOC->MODER);
      log_i("[dbg] GPIOA_AFRL=0x%08lX GPIOC_AFRL=0x%08lX GPIOC_AFRH=0x%08lX",
            GPIOA->AFR[0], GPIOC->AFR[0], GPIOC->AFR[1]);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while(1)
    {
    /* USER CODE END WHILE */
//    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
      if(dma_tx_done)
      {

          dma_tx_done = 0;
          cs43lxxx_status_t i2s_ret = g_cs43lxxx_hal_ops.pf_i2s_transmit_with_dma(
                                      (uint16_t *)sine_1khz, 96);
          log_i("[i2s] DMA TX complete!");                                      
      }
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
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 200;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 5;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
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
        dma_tx_done = 1;
        // log_i("[i2s] TxCpltCallback: DMA transfer complete!");
    }
}

/* I2S DMA TX half-complete callback */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if(hi2s == &hi2s3)
    {
        // log_i("[i2s] TxHalfCpltCallback: half done");
    }
}

/* I2S error callback */
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if(hi2s == &hi2s3)
    {
        uint32_t err = HAL_I2S_GetError(hi2s);
        log_i("[i2s] ErrorCallback: err=0x%08lX", err);
    }
}
/* USER CODE END 4 */

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
