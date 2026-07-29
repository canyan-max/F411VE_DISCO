/**
 ******************************************************************************
 * @file    : audio_out.c
 * @brief   : Audio output device abstraction.
 *            All BSP-specific references are confined to this file.
 *            To swap the codec, only this file needs updating.
 * @version : V1.0  2026
 * @note    : 1 tab == 4 spaces!
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "audio_out.h"
#include "cs43lxxx_hal.h"
#include "bsp_cs43lxxx_drv.h"
#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif

/* private variables --------------------------------------------------------*/
typedef struct
{
    cs43xxx_drv_t            *p_drv;
    const audio_out_cb_cfg_t *p_cb;
} audio_out_ctx_t;

static audio_out_ctx_t s_ctx = {NULL, NULL};

/* exported functions -------------------------------------------------------*/
platform_err_t audio_out_init(const audio_out_cb_cfg_t *p_cb)
{
    if (p_cb == NULL)
    {
#ifdef USER_DEBUG_LOG
        log_e("[audio_out] init param err");
#endif
        return PLATFORM_ERR_PARAM;
    }
    s_ctx.p_drv = &g_cs43l22_drv;
    s_ctx.p_cb  = p_cb;

    cs43lxxx_status_t ret = cs43lxxx_instruct(s_ctx.p_drv,
                                               &g_cs43lxxx_hal_ops,
                                               CS43XXX_I2C_ADDR_7BIT,
                                               OUTPUT_DEVICE_AUTO);
    if (ret != CS43LXXX_STATUS_OK)
    {
#ifdef USER_DEBUG_LOG
        log_e("[audio_out] codec init failed ret=%d", ret);
#endif
        return PLATFORM_ERR_FAIL;
    }
    return PLATFORM_ERR_OK;
}

platform_err_t audio_out_start(int16_t *p_buf, uint16_t len)
{
    if (p_buf == NULL || s_ctx.p_drv == NULL)
    {
#ifdef USER_DEBUG_LOG
        log_e("[audio_out] start param err");
#endif
        return PLATFORM_ERR_PARAM;
    }
    s_ctx.p_drv->p_hal_ops->pf_i2s_stop_dma();

    cs43lxxx_status_t ret = s_ctx.p_drv->pf_play(s_ctx.p_drv);
    if (ret != CS43LXXX_STATUS_OK)
    {
#ifdef USER_DEBUG_LOG
        log_e("[audio_out] play failed");
#endif
        return PLATFORM_ERR_FAIL;
    }
    ret = s_ctx.p_drv->p_hal_ops->pf_i2s_transmit_with_dma((uint16_t *)p_buf,
                                                             len);
    return (ret == CS43LXXX_STATUS_OK) ? PLATFORM_ERR_OK : PLATFORM_ERR_FAIL;
}

void audio_out_stop(void)
{
    if (s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_stop(s_ctx.p_drv);
    }
}

void audio_out_soft_stop(void)
{
    if (s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_soft_stop(s_ctx.p_drv);
    }
}

void audio_out_play(void)
{
    if (s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_play(s_ctx.p_drv);
    }
}

void audio_out_pause(void)
{
    if (s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_pause(s_ctx.p_drv);
    }
}

void audio_out_resume(void)
{
    if (s_ctx.p_drv != NULL)
    {
        s_ctx.p_drv->pf_resume(s_ctx.p_drv);
    }
}

platform_err_t audio_out_set_sample_rate(uint32_t hz)
{
    if (s_ctx.p_drv == NULL || s_ctx.p_drv->p_hal_ops == NULL)
    {
        return PLATFORM_ERR_NOT_INIT;
    }
    cs43lxxx_status_t ret = s_ctx.p_drv->p_hal_ops->pf_i2s_set_audio_freq(hz);
    return (ret == CS43LXXX_STATUS_OK) ? PLATFORM_ERR_OK : PLATFORM_ERR_FAIL;
}

platform_err_t audio_out_set_volume(uint8_t vol)
{
    if (s_ctx.p_drv == NULL)
    {
        return PLATFORM_ERR_NOT_INIT;
    }
    cs43lxxx_status_t ret = s_ctx.p_drv->pf_set_volume(s_ctx.p_drv, vol);
    return (ret == CS43LXXX_STATUS_OK) ? PLATFORM_ERR_OK : PLATFORM_ERR_FAIL;
}

void audio_out_tx_half_cplt(void)
{
    if (s_ctx.p_cb != NULL && s_ctx.p_cb->pf_half_cplt != NULL)
    {
        s_ctx.p_cb->pf_half_cplt();
    }
}

void audio_out_tx_cplt(void)
{
    if (s_ctx.p_cb != NULL && s_ctx.p_cb->pf_cplt != NULL)
    {
        s_ctx.p_cb->pf_cplt();
    }
}

/* end of file --------------------------------------------------------------*/
