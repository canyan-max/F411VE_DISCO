/**
 ******************************************************************************
 * @file    : device_audio_out.h
 * @brief   : Audio output device abstraction.
 *            Stable, BSP-agnostic interface. No BSP types appear here.
 *            To swap the codec, only device_audio_out.c needs updating.
 * @version : V1.0  2026
 * @note    : 1 tab == 4 spaces!
 ******************************************************************************
 */
#ifndef DEVICE_AUDIO_OUT_H
#define DEVICE_AUDIO_OUT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include <stdint.h>
#include "platform_error.h"

/* typedef ------------------------------------------------------------------*/
typedef void (*audio_out_half_cb_t)(void);
typedef void (*audio_out_cplt_cb_t)(void);

typedef struct
{
    audio_out_half_cb_t pf_half_cplt;
    audio_out_cplt_cb_t pf_cplt;
} audio_out_cb_cfg_t;

/* functions ----------------------------------------------------------------*/
platform_err_t audio_out_init(const audio_out_cb_cfg_t *p_cb);
platform_err_t audio_out_start(int16_t *p_buf, uint16_t len);
platform_err_t audio_out_set_sample_rate(uint32_t hz);
platform_err_t audio_out_set_volume(uint8_t vol);
void           audio_out_stop(void);
void           audio_out_soft_stop(void);
void           audio_out_play(void);
void           audio_out_pause(void);
void           audio_out_resume(void);
void           audio_out_tx_half_cplt(void);
void           audio_out_tx_cplt(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_AUDIO_OUT_H */
