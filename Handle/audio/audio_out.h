/**
 ******************************************************************************
 * @file    : audio_out.h
 * @brief   : Audio output handle layer.
 *            Stable, BSP-agnostic interface between mp3_player and the
 *            codec driver.  No BSP types appear in this header.
 * @version : V1.0  2026
 ******************************************************************************
 */
#ifndef AUDIO_OUT_H
#define AUDIO_OUT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include <stdint.h>

/* typedef ------------------------------------------------------------------*/
typedef enum
{
    AUDIO_OUT_OK = 0,
    AUDIO_OUT_ERROR,
} audio_out_status_t;

typedef void (*audio_out_half_cb_t)(void);
typedef void (*audio_out_cplt_cb_t)(void);

typedef struct
{
    audio_out_half_cb_t pf_half_cplt;
    audio_out_cplt_cb_t pf_cplt;
} audio_out_cb_cfg_t;

/* functions ----------------------------------------------------------------*/

/* One-time init: initialises the codec and registers the DMA callbacks.
 * Must be called before audio_out_start. */
audio_out_status_t audio_out_init(const audio_out_cb_cfg_t *p_cb);

/* Start circular DMA playback.  p_buf must already contain valid PCM data.
 * Internally calls pf_play then starts I2S DMA. */
audio_out_status_t audio_out_start(int16_t *p_buf, uint16_t len);

void               audio_out_stop(void);
void               audio_out_pause(void);
void               audio_out_resume(void);
audio_out_status_t audio_out_set_volume(uint8_t vol);

/* Call from HAL_I2S_TxHalfCpltCallback / HAL_I2S_TxCpltCallback */
void audio_out_tx_half_cplt(void);
void audio_out_tx_cplt(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_OUT_H */
