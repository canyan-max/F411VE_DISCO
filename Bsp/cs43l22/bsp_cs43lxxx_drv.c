/**
 ******************************************************************************
 *@file               :   bsp_cs43lxxx_drv.c
 *
 *@brief              :   Provide the HAL APIs of description.
 *
 *@version            :   V1.0
 *
 *@note               :   1 tab == 4 spaces!  2026
 *
 *@pardependencies    :   bsp_cs43lxxx_drv.c
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <stddef.h>           /* stdint lib header file. */
#include "bsp_cs43lxxx_drv.h" /* bsp_cs43lxxx_drv lib header file. */
#include "cs43lxxx_regmap.h"  /* cs43lxxx_regmap lib header file. */
/* define   -----------------------------------------------------------------*/
#define CS43LXXX_READ_REG(dev, reg, buf, len) \
    ((dev)->p_hal_ops->pf_i2c_read_reg((dev)->dev_i2c_adr, (reg), (buf), (len)))

#define CS43LXXX_WRITE_REG(dev, reg, buf, len) \
    ((dev)->p_hal_ops->pf_i2c_write_reg((dev)->dev_i2c_adr, (reg), (buf), (len)))

#define VOLUME_CONVERT(Volume)    \
    (((Volume) > 100)? 255:((uint8_t)(((Volume) * 255) / 100)))

#define MUTE_DISABLE        (0x00U)
#define MUTE_ENABLE         (0x01U)
/* typedef ------------------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/

/* private  functions  ------------------------------------------------------*/

/**
 * @brief            :  [cs43lxxx_read_id]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t      *p_drv ,uint8_t *p_id]
 */
static cs43lxxx_status_t cs43lxxx_read_id(cs43xxx_drv_t *p_drv, uint8_t *p_id)
{
    if(NULL == p_drv || NULL == p_id)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    uint8_t           id  = 0x00;
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;
    ret                   = CS43LXXX_READ_REG(p_drv, CS43L22_REG_ID, &id, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    id = (id & CS43L22_CHIP_ID_MASK);
    if(CS43L22_CHIP_ID != id)
    {
        *p_id = 0xa5;
        return CS43LXXX_STATUS_ERROR;
    }
    *p_id = id;
    return CS43LXXX_STATUS_OK;
}
/**
 * @brief            :  [cs43lxxx_set_volume]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_set_volume(cs43xxx_drv_t *p_drv,
                                             uint8_t        volume)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }

    uint8_t           convertedvol    = VOLUME_CONVERT(volume);
    cs43lxxx_status_t ret             = CS43LXXX_STATUS_OK;
    uint8_t           temp_set_volume = 0;

    if(convertedvol > 0xE6)
    {
        temp_set_volume = convertedvol - 0xE7U;
    }
    else
    {
        temp_set_volume = convertedvol + 0x19U;
    }

    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_MASTER_A_VOL, &temp_set_volume,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_MASTER_B_VOL, &temp_set_volume,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_set_mute]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_set_mute(cs43xxx_drv_t *p_drv)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }

    cs43lxxx_status_t ret      = CS43LXXX_STATUS_OK;
    uint8_t           temp_reg = 0xFFU;

    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL2, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    temp_reg = 0x01U;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_HEADPHONE_A_VOL, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_HEADPHONE_B_VOL, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_set_mute]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_set_out(cs43xxx_drv_t *p_drv)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }

    cs43lxxx_status_t ret      = CS43LXXX_STATUS_OK;
    uint8_t           temp_reg = 0x00U;

    /* Unmute headphone outputs (0x00 = 0dB, not muted). */
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_HEADPHONE_A_VOL, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_HEADPHONE_B_VOL, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    /* Restore device-dependent POWER_CTL2 (per ST BSP SetMute OFF). */
    switch(p_drv->out_put)
    {
        case OUTPUT_DEVICE_SPEAKER:
            temp_reg = 0xFA;
            break;
        case OUTPUT_DEVICE_HEADPHONE:
            temp_reg = 0xAF;
            break;
        case OUTPUT_DEVICE_BOTH:
            temp_reg = 0xAA;
            break;
        case OUTPUT_DEVICE_AUTO:
        default:
            temp_reg = 0x05;
            break;
    }
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL2, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_play]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_play(cs43xxx_drv_t *p_drv)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;
    uint8_t temp_reg      = 0x06U; /* Enable digital soft-ramp (per ST BSP) */

    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_MISC_CTL, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    ret = cs43lxxx_set_out(p_drv);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    temp_reg = 0x9EU;
    ret      = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL1, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_stop]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_stop(cs43xxx_drv_t *p_drv)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;
    uint8_t           temp_reg;

    /* 1. Mute outputs while MCLK is still present */
    ret = cs43lxxx_set_mute(p_drv);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    /* 2. Power down codec (PDN) while MCLK is still present so the
     *    charge pump and DAC can shut down cleanly */
    temp_reg = 0x04U;
    ret      = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL1, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    temp_reg = 0x9FU;
    ret      = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL1, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    /* 3. Now stop I2S DMA — MCLK disappears after codec is already down */
    ret = p_drv->p_hal_ops->pf_i2s_stop_dma();
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    /* 4. Pull RESET low for a fully clean hardware state */
    p_drv->p_hal_ops->pf_delay_ms(1);
    p_drv->p_hal_ops->pf_power_control(0);
    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_resume]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_resume(cs43xxx_drv_t *p_drv)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;
    uint8_t temp_reg = 0x9EU;
    volatile uint32_t index = 0x00;
    /* Unmute (restore POWER_CTL2 + HP volume), then restart DMA.
     * POWER_CTL1 is not changed — codec stayed at 0x9E through pause. */
    ret = cs43lxxx_set_out(p_drv);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    for(index = 0x00; index < 0xFF; index++);
    
//    ret      = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL1, &temp_reg, 1);
//    if(CS43LXXX_STATUS_OK != ret)
//    {
//        return ret;
//    }
    ret = p_drv->p_hal_ops->pf_i2s_resume_dma();
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_resume]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv,
                         uint8_t        volume]
 */
static cs43lxxx_status_t cs43lxxx_pause(cs43xxx_drv_t *p_drv)
{
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;
    uint8_t temp_reg = 0x01U;
    
    ret = p_drv->p_hal_ops->pf_i2s_pause_dma();
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    /* Mute outputs, then pause DMA. POWER_CTL1 is intentionally not changed —
     * codec remains at 0x9E so it can resume cleanly without re-init. */
    ret = cs43lxxx_set_mute(p_drv);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    
    ret      = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL1, &temp_reg, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    return CS43LXXX_STATUS_OK;
}

/**
 * @brief            :  [cs43lxxx_init]
 * @retval           :  [    // CS43LXXX I2C operations is OK.
                                CS43LXXX_STATUS_OK = 0x00U,
                            //  CS43LXXX I2C operations is ERROR.
                                CS43LXXX_STATUS_ERROR,
                                //CS43LXXX I2C operations is BUSY.
                                CS43LXXX_STATUS_BUSY,
                            //  CS43LXXX I2C operations is TIMEOUT.
                                CS43LXXX_STATUS_TIMEOUT,
                                // CS43LXXX_STATUS_ERR_SRC
                                CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t *p_drv]
 */
static cs43lxxx_status_t cs43lxxx_init(cs43xxx_drv_t *p_drv, uint8_t volume)
{
    uint8_t           reg_value = 0x00;
    cs43lxxx_status_t ret       = CS43LXXX_STATUS_OK;
    if(NULL == p_drv)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    // step 0 power reset 
    p_drv->p_hal_ops->pf_power_control(0);
    p_drv->p_hal_ops->pf_delay_ms(5);
    p_drv->p_hal_ops->pf_power_control(1);
    p_drv->p_hal_ops->pf_delay_ms(10);
    // step 1
    reg_value = 0x01;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL1, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 2: Device-dependent power config (per ST BSP)
    switch(p_drv->out_put)
    {
        case OUTPUT_DEVICE_SPEAKER:
            reg_value = 0xFA; /* SPK always ON, HP always OFF */
            break;
        case OUTPUT_DEVICE_HEADPHONE:
            reg_value = 0xAF; /* SPK always OFF, HP always ON */
            break;
        case OUTPUT_DEVICE_BOTH:
            reg_value = 0xAA; /* SPK always ON, HP always ON */
            break;
        case OUTPUT_DEVICE_AUTO:
            reg_value = 0x05; /* Auto-detect HP or SPK */
            break;
        default:
            reg_value = 0x05; /* Auto-detect HP or SPK */
            break;
    }
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_POWER_CTL2, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 3: Auto speed detection, External MCLK (per ST BSP)
    reg_value = 0x81;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_CLOCKING_CTL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 4
    reg_value = 0x04;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_INTERFACE_CTL1, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 4a: INTERFACE_CTL2 — SCLK normal polarity, no invert
    reg_value = 0x00;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_INTERFACE_CTL2, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 4b: Disable analog passthrough, use I2S→DAC path
    reg_value = 0x00;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PASSTHR_A_SELECT, &reg_value,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    reg_value = 0x00;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PASSTHR_B_SELECT, &reg_value,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 5
    reg_value = volume;
    ret       = cs43lxxx_set_volume(p_drv, reg_value);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 5a: Passthrough gang control — independent L/R
    reg_value = 0x00;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PASSTHR_GANG_CTL, &reg_value,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 5b: PLAYBACK_CTL1 — unmute, soft-ramp off
    reg_value = 0x00;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PLAYBACK_CTL1, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 6
    reg_value = 0x06U;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PLAYBACK_CTL2, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 7
    reg_value = 0x00U;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_SPEAKER_A_VOL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_SPEAKER_B_VOL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }

    // step 8
    reg_value = 0x00U;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_ANALOG_ZC_SR_SETT, &reg_value,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 9
    reg_value = 0x04U;
    ret       = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_MISC_CTL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 10
    reg_value = 0x00U;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_LIMIT_CTL1, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 11
    reg_value = 0x0FU;
    ret       = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_TONE_CTL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 12
    reg_value = 0x00U; /* 0dB, no attenuation */
    ret       = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PCMA_VOL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_PCMB_VOL, &reg_value, 1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    // step 13: Enable charge pump (required for headphone output)
    reg_value = 0x05U;
    ret = CS43LXXX_WRITE_REG(p_drv, CS43L22_REG_CHARGE_PUMP_FREQ, &reg_value,
                             1);
    if(CS43LXXX_STATUS_OK != ret)
    {
        return ret;
    }
    return CS43LXXX_STATUS_OK;
}
/* exported functions -------------------------------------------------------*/
/**
 * @brief            :  [cs43lxxx_instruct]
 * @retval           :  [ CS43LXXX_STATUS_OK = 0x00U,
                            CS43LXXX_STATUS_ERROR,
                            CS43LXXX_STATUS_BUSY,
                            CS43LXXX_STATUS_TIMEOUT,
                            CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t      *p_drv,
                                    cs43lxxx_hal_ops_t *p_hal_ops,
                                    uint8_t             i2c_id]
 */
cs43lxxx_status_t cs43lxxx_instruct(cs43xxx_drv_t      *p_drv,
                                    cs43lxxx_hal_ops_t *p_hal_ops,
                                    uint8_t             i2c_id,
                                    uint8_t             out_put_dev)
{
    if(NULL == p_drv || NULL == p_hal_ops)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    p_drv->p_hal_ops   = p_hal_ops;
    p_drv->dev_i2c_adr = i2c_id;
    p_drv->out_put     = out_put_dev;

    /* Mount self operation function pointers. */
    p_drv->pf_init       = cs43lxxx_init;
    p_drv->pf_read_id    = cs43lxxx_read_id;
    p_drv->pf_set_volume = cs43lxxx_set_volume;
    p_drv->pf_set_mute   = cs43lxxx_set_mute;
    p_drv->pf_set_out    = cs43lxxx_set_out;
    p_drv->pf_play       = cs43lxxx_play;
    p_drv->pf_stop       = cs43lxxx_stop;
    p_drv->pf_resume     = cs43lxxx_resume;
    p_drv->pf_pause      = cs43lxxx_pause;
        /* Try to initialize the device once. */
        cs43lxxx_status_t ret = p_drv->pf_init(p_drv, 70);
    if(CS43LXXX_STATUS_OK != ret)
    {
        /* Init failed, clear all self ops and mark as not initialized. */
        p_drv->pf_init       = NULL;
        p_drv->pf_read_id    = NULL;
        p_drv->pf_set_volume = NULL;
        p_drv->pf_set_mute   = NULL;
        p_drv->pf_set_out    = NULL;
        p_drv->pf_play       = NULL;
        p_drv->pf_stop       = NULL;
        p_drv->pf_resume     = NULL;
        p_drv->pf_pause      = NULL;
        p_drv->is_init       = CS43XXX_NOT_INIT;
        return ret;
    }

    p_drv->is_init = CS43XXX_IS_INIT;
    return CS43LXXX_STATUS_OK;
}
/* end of  file -------------------------------------------------------------*/
