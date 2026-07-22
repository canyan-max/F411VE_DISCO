/**
 ******************************************************************************
 * @file    : audio_out.c
 * @brief   : Audio output handle layer.
 *            All BSP-specific references are confined to this file.
 *            To swap the codec BSP, only audio_out_init() needs updating.
 * @version : V1.0  2026
 *  @note   : 1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "audio_out.h"
#include "cs43lxxx_hal.h"     /* g_cs43lxxx_hal_ops, g_cs43l22_drv          */
#include "bsp_cs43lxxx_drv.h" /* cs43lxxx_instruct, CS43XXX_I2C_ADDR_7BIT   */

#define AUDIO_DBG
#ifdef  AUDIO_DBG
#define AOUT_TAG "aout"
#include "elog.h"
#endif // end of AUDIO_DBG

/* Internal context — every function goes through s_ctx, so the BSP type
 * and instance name appear only in audio_out_init().                        */
typedef struct
{
    cs43xxx_drv_t            *p_drv;
    const audio_out_cb_cfg_t *p_cb;
} audio_out_ctx_t;

static audio_out_ctx_t s_ctx = {NULL, NULL};

/* exported functions -------------------------------------------------------*/
/**
 * @brief            :  [audio_out_init]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [const audio_out_cb_cfg_t *p_cb]
 */
audio_out_status_t audio_out_init(const audio_out_cb_cfg_t *p_cb)
{
    if(p_cb == NULL)
    {
#ifdef AUDIO_DBG
       log_e("audio_out_init input arg err");
#endif // end of AUDIO_DBG 
        return AUDIO_OUT_ERROR;
    }
        

    /* Bind BSP instances — only place in this file that names them directly */
    s_ctx.p_drv = &g_cs43l22_drv;
    s_ctx.p_cb  = p_cb;

    cs43lxxx_status_t ret = cs43lxxx_instruct(s_ctx.p_drv,
                                               &g_cs43lxxx_hal_ops,
                                               CS43XXX_I2C_ADDR_7BIT,
                                               OUTPUT_DEVICE_AUTO);
    if(ret != CS43LXXX_STATUS_OK)
    {
#ifdef AUDIO_DBG
       log_e("instruct failed ret=%d", ret);
#endif // end of AUDIO_DBG

        return AUDIO_OUT_ERROR;
    }
#ifdef AUDIO_DBG
    log_i("init ok is_init=%d", s_ctx.p_drv->is_init);
#endif // end of AUDIO_DBG

    return AUDIO_OUT_OK;
}

/**
 * @brief            :  [audio_out_start]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [int16_t *p_buf, uint16_t len]
 */
audio_out_status_t audio_out_start(int16_t *p_buf, uint16_t len)
{
    if(p_buf == NULL || s_ctx.p_drv == NULL)
    {
#ifdef AUDIO_DBG
        log_e("audio_out_start input arg err");
#endif // end of AUDIO_DBG
        return AUDIO_OUT_ERROR;
    }


    // stop the dma 
    s_ctx.p_drv->p_hal_ops->pf_i2s_stop_dma();

    cs43lxxx_status_t ret = s_ctx.p_drv->pf_play(s_ctx.p_drv);
    if(ret != CS43LXXX_STATUS_OK)
    {
#ifdef AUDIO_DBG
        log_e("pf_play failed ret=%d", ret);
#endif // end of AUDIO_DBG
        return AUDIO_OUT_ERROR;
    }

    ret = s_ctx.p_drv->p_hal_ops->pf_i2s_transmit_with_dma((uint16_t *)p_buf, len);
    if(ret != CS43LXXX_STATUS_OK)
    {
#ifdef AUDIO_DBG
        log_e("i2s transmit failed ret=%d", ret);
#endif // end of AUDIO_DBG
        return AUDIO_OUT_ERROR;
    }

    return AUDIO_OUT_OK;
}

/**
 * @brief            :  [audio_out_stop]
 */
void audio_out_stop(void)
{
    if(s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_stop(s_ctx.p_drv);
    }
}

/**
 * @brief            :  [audio_out_soft_stop]
 */
void audio_out_soft_stop(void)
{
    if(s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_soft_stop(s_ctx.p_drv);
    }
}

/**
 * @brief            :  [audio_out_play]
 */
void audio_out_play(void)
{
    if(s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_play(s_ctx.p_drv);
    }
}

/**
 * @brief            :  [audio_out_pause]
 */
void audio_out_pause(void)
{
    if(s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_pause(s_ctx.p_drv);
    }
}

/**
 * @brief            :  [audio_out_resume]
 */
void audio_out_resume(void)
{
    if(s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_resume(s_ctx.p_drv);
    }
}

/**
 * @brief            :  [audio_out_set_sample_rate]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [uint32_t hz]
 */
audio_out_status_t audio_out_set_sample_rate(uint32_t hz)
{
    if(s_ctx.p_drv == NULL || s_ctx.p_drv->p_hal_ops == NULL)
    {
#ifdef AUDIO_DBG
        log_e("audio_out_set_sample_rate input arg err");
#endif // end of AUDIO_DBG
        return AUDIO_OUT_ERROR;
    }

    cs43lxxx_status_t ret = s_ctx.p_drv->p_hal_ops->pf_i2s_set_audio_freq(hz);
    return (ret == CS43LXXX_STATUS_OK) ? AUDIO_OUT_OK : AUDIO_OUT_ERROR;
}
/**
 * @brief            :  [audio_out_set_volume]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [uint8_t vol]
 */
audio_out_status_t audio_out_set_volume(uint8_t vol)
{
    if(s_ctx.p_drv == NULL)
    {
#ifdef AUDIO_DBG
        log_e("audio_out_set_volume input arg err");
#endif // end of AUDIO_DBG
        return AUDIO_OUT_ERROR;
    }

    cs43lxxx_status_t ret = s_ctx.p_drv->pf_set_volume(s_ctx.p_drv, vol);
    return (ret == CS43LXXX_STATUS_OK) ? AUDIO_OUT_OK : AUDIO_OUT_ERROR;
}


/**
 * @brief            :  [audio_out_tx_half_cplt]
 */
void audio_out_tx_half_cplt(void)
{
    if(s_ctx.p_cb != NULL && s_ctx.p_cb->pf_half_cplt != NULL)
    {
        s_ctx.p_cb->pf_half_cplt();
    }
}

/**
 * @brief            :  [audio_out_tx_cplt]
 */
void audio_out_tx_cplt(void)
{
    if(s_ctx.p_cb != NULL && s_ctx.p_cb->pf_cplt != NULL)
    {
        s_ctx.p_cb->pf_cplt();
    }
}
/* end of file --------------------------------------------------------------*/
