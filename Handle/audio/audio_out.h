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
/**
 * @brief            :  [audio_out_init]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [const audio_out_cb_cfg_t *p_cb]
 */
audio_out_status_t audio_out_init(const audio_out_cb_cfg_t *p_cb);
/**
 * @brief            :  [audio_out_start]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [int16_t *p_buf, uint16_t len]
 */
audio_out_status_t audio_out_start(int16_t *p_buf, uint16_t len);
/**
 * @brief            :  [audio_out_stop]
 */
void audio_out_stop(void);
/**
 * @brief            :  [audio_out_soft_stop]
 */
void audio_out_soft_stop(void);
/**
 * @brief            :  [audio_out_play]
 */
void audio_out_play(void);
/**
 * @brief            :  [audio_out_pause]
 */
void audio_out_pause(void);
/**
 * @brief            :  [audio_out_resume]
 */
void audio_out_resume(void);
/**
 * @brief            :  [audio_out_set_sample_rate]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [uint32_t hz]
 */
audio_out_status_t audio_out_set_sample_rate(uint32_t hz);
/**
 * @brief            :  [audio_out_set_volume]
 * @retval           :  [    AUDIO_OUT_OK = 0,
                             AUDIO_OUT_ERROR,]
 * @param[in]        :  [uint8_t vol]
 */
audio_out_status_t audio_out_set_volume(uint8_t vol);


/**
 * @brief            :  [audio_out_tx_half_cplt]
 */
void audio_out_tx_half_cplt(void);
/**
 * @brief            :  [audio_out_tx_half_cplt]
 */
void audio_out_tx_cplt(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_OUT_H */
