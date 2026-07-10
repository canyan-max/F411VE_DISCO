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
#include <stddef.h>       /* stdint lib header file. */
#include "cs43lxxx_hal.h" /* cs43lxxx_hal lib header file. */
#include "i2c.h"
#include "i2s.h"
#include "bsp_cs43lxxx_drv.h" /* bsp_cs43lxxx_drv lib header file. */
/* define   -----------------------------------------------------------------*/
#define CS43LXXX_I2C_DELAY_MS  (10U) /* CS43LXXX I2C delay ms. */
/* typedef ------------------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/
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
static void              cs43lxxx_hal_delay_ms(uint32_t ms);

static void cs43lxxx_hal_power_control(uint8_t state);
cs43lxxx_hal_ops_t g_cs43lxxx_hal_ops =
    {.pf_i2c_read_reg          = cs43lxxx_hal_i2c_read_reg,
     .pf_i2c_write_reg         = cs43lxxx_hal_i2c_write_reg,
     .pf_i2s_transmit_with_dma = cs43lxxx_hal_i2s_transmit_dma,
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
