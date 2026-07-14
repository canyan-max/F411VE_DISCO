/**
 ******************************************************************************
 *@file               :   cs43lxxx_hal.c
 *
 *@brief              :   Provide the HAL APIs of description.
 *
 *@version            :   V1.0
 *
 *@note               :   1 tab == 4 spaces!  2026
 *
 *@pardependencies    :   cs43lxxx_hal.c
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "cs43lxxx_hal.h" /* cs43lxxx_hal lib header file. */
#include "i2c.h"
#include "i2s.h"
#include "bsp_cs43lxxx_drv.h" /* bsp_cs43lxxx_drv lib header file. */
/* define   -----------------------------------------------------------------*/
#define CS43LXXX_I2C_DELAY_MS  (10U) /* CS43LXXX I2C delay ms. */
/* typedef ------------------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/
cs43xxx_drv_t g_cs43l22_drv = {0};

static cs43lxxx_status_t cs43lxxx_hal_i2c_write_reg(uint8_t  dev_addr,
                                                    uint16_t reg_addr,
                                                    uint8_t *p_data,
                                                    uint16_t len);
static cs43lxxx_status_t cs43lxxx_hal_i2c_read_reg(uint8_t  dev_addr,
                                                   uint16_t reg_addr,
                                                   uint8_t *p_data,
                                                   uint16_t len);
static cs43lxxx_status_t cs43lxxx_hal_i2s_transmit_dma(uint16_t *p_buffer,
                                                       uint16_t  size);
static cs43lxxx_status_t cs43lxxx_hal_i2s_dma_stop(void);
static cs43lxxx_status_t cs43lxxx_hal_i2s_dma_resume(void);
static cs43lxxx_status_t cs43lxxx_hal_i2s_dma_pause(void);
static void              cs43lxxx_hal_delay_ms(uint32_t ms);
static void              cs43lxxx_hal_power_control(uint8_t state);

cs43lxxx_hal_ops_t g_cs43lxxx_hal_ops =
    {.pf_i2c_read_reg          = cs43lxxx_hal_i2c_read_reg,
     .pf_i2c_write_reg         = cs43lxxx_hal_i2c_write_reg,
     .pf_i2s_transmit_with_dma = cs43lxxx_hal_i2s_transmit_dma,
     .pf_i2s_stop_dma          = cs43lxxx_hal_i2s_dma_stop,
     .pf_i2s_resume_dma        = cs43lxxx_hal_i2s_dma_resume,
     .pf_i2s_pause_dma         = cs43lxxx_hal_i2s_dma_pause,
     .pf_delay_ms              = cs43lxxx_hal_delay_ms,
     .pf_power_control         = cs43lxxx_hal_power_control};

/* private  functions  ------------------------------------------------------*/

/**
 * @brief            :  [cs43lxxx_hal_i2c_write_reg]
 * @retval           :  [
                            CS43LXXX_STATUS_OK = 0x00U,
                            CS43LXXX_STATUS_ERROR,
                            CS43LXXX_STATUS_BUSY,
                            CS43LXXX_STATUS_TIMEOUT
                            ]
 * @param[in]        :  [uint8_t  dev_addr,
                         uint16_t reg_addr,
                         uint8_t *p_data,
                         uint16_t len]
 */
static cs43lxxx_status_t cs43lxxx_hal_i2c_write_reg(uint8_t  dev_addr,
                                                    uint16_t reg_addr,
                                                    uint8_t *p_data,
                                                    uint16_t len)
{
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg_addr,
                                              I2C_MEMADD_SIZE_8BIT, p_data, len,
                                              CS43LXXX_I2C_DELAY_MS);
    if(HAL_OK != ret)
    {
        return CS43LXXX_STATUS_ERROR;
    }
    return CS43LXXX_STATUS_OK;
}
/**
 * @brief            :  [cs43lxxx_hal_i2c_read_reg]
 * @retval           :  [CS43LXXX_STATUS_OK = 0x00U,
                         CS43LXXX_STATUS_ERROR,
                         CS43LXXX_STATUS_BUSY,
                         CS43LXXX_STATUS_TIMEOUT
                        ]
 * @param[in]        :  [uint8_t  dev_addr,
                         uint16_t reg_addr,
                         uint16_t len]
 * @param[out]        :  [uint8_t *p_data]
 */
static cs43lxxx_status_t cs43lxxx_hal_i2c_read_reg(uint8_t  dev_addr,
                                                   uint16_t reg_addr,
                                                   uint8_t *p_data,
                                                   uint16_t len)
{
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg_addr,
                                             I2C_MEMADD_SIZE_8BIT, p_data, len,
                                             CS43LXXX_I2C_DELAY_MS);
    if(HAL_OK != ret)
    {
        return CS43LXXX_STATUS_ERROR;
    }
    return CS43LXXX_STATUS_OK;
}
/**
 * @brief            :  [cs43lxxx_hal_i2s_transmit_dma]
 * @retval           :  [CS43LXXX_STATUS_OK = 0x00U,
                         CS43LXXX_STATUS_ERROR,
                         CS43LXXX_STATUS_BUSY,
                         CS43LXXX_STATUS_TIMEOUT
                        ]
 * @param[in]        :  [uint16_t *p_buffer,
                         uint16_t  size]
 */
static cs43lxxx_status_t cs43lxxx_hal_i2s_transmit_dma(uint16_t *p_buffer,
                                                       uint16_t  size)
{
    HAL_StatusTypeDef ret = HAL_I2S_Transmit_DMA(&hi2s3, p_buffer, size);
    if(HAL_OK != ret)
    {
        return CS43LXXX_STATUS_ERROR;
    }
    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_hal_i2s_dma_resume]
 * @retval           :  [CS43LXXX_STATUS_OK = 0x00U,
                         CS43LXXX_STATUS_ERROR,
                         CS43LXXX_STATUS_BUSY,
                         CS43LXXX_STATUS_TIMEOUT
                        ]
 */
static cs43lxxx_status_t cs43lxxx_hal_i2s_dma_resume(void)
{
    /* HAL_I2S_DMAResume() 内部会多余地使能 TXE 中断，在 DMA 模式下
     * 会触发中断处理函数与 DMA 冲突并破坏 hi2s3.State，导致下次
     * Resume 失败。直接操作 TXDMAEN 位可靠且无副作用。 */
    SET_BIT(SPI3->CR2, SPI_CR2_TXDMAEN);
    return CS43LXXX_STATUS_OK;
}
/**
 * @brief            :  [cs43lxxx_hal_i2s_dma_pause]
 * @retval           :  [CS43LXXX_STATUS_OK = 0x00U,
                         CS43LXXX_STATUS_ERROR,
                         CS43LXXX_STATUS_BUSY,
                         CS43LXXX_STATUS_TIMEOUT
                        ]
 */
static cs43lxxx_status_t cs43lxxx_hal_i2s_dma_pause(void)
{
    /* 直接清 TXDMAEN 位——DMA 流本身保持使能（循环模式），
     * MCLK/BCLK/LRCK 继续，仅停止 I2S 向 DMA 发出请求。 */
    CLEAR_BIT(SPI3->CR2, SPI_CR2_TXDMAEN);
    return CS43LXXX_STATUS_OK;
}
/**
 * @brief            :  [cs43lxxx_hal_i2s_dma_stop]
 * @retval           :  [CS43LXXX_STATUS_OK = 0x00U,
                         CS43LXXX_STATUS_ERROR,
                         CS43LXXX_STATUS_BUSY,
                         CS43LXXX_STATUS_TIMEOUT
                        ]
 */
static cs43lxxx_status_t cs43lxxx_hal_i2s_dma_stop(void)
{
    HAL_StatusTypeDef ret = HAL_I2S_DMAStop(&hi2s3);
    if(HAL_OK != ret)
    {
        return CS43LXXX_STATUS_ERROR;
    }
    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_hal_power_control]
 * @param[in]        :  [uint8_t state 1 : power on, 0 : power off]
 */
static void cs43lxxx_hal_power_control(uint8_t state)
{
    if(state)
    {
        HAL_GPIO_WritePin(Audio_RST_GPIO_Port, Audio_RST_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(Audio_RST_GPIO_Port, Audio_RST_Pin, GPIO_PIN_RESET);
    }
}
/**
 * @brief            :  [cs43lxxx_hal_delay_ms]
 * @param[in]        :  [uint32_t ms]
 */
static void cs43lxxx_hal_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
/* exported functions -------------------------------------------------------*/

/* end of  file -------------------------------------------------------------*/
