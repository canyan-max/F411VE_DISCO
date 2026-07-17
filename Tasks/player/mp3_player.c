/**
 ******************************************************************************
 *@file               :   mp3_player.c
 *
 *@brief              :   Provide the HAL APIs of description.
 *
 *@version            :   V1.0
 *
 *@note               :   1 tab == 4 spaces!  2026
 *
 *@pardependencies    :   mp3_player.c
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <stddef.h>     /* stdint lib header file. */
#include "stm32f4xx.h" /* __disable_irq / __enable_irq (CMSIS) */
#include "mp3_player.h" /* mp3_player lib header file. */

// #define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_ONLY_MP3     1
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "audio_out.h"
#include <string.h>

#define MP3_PLARER_DBG
#ifdef  MP3_PLARER_DBG
#define MP3_PLARERTAG "mp3player"
#include "elog.h"
#endif // end of MP3_PLARER_DBG
/* define   -----------------------------------------------------------------*/
/* Each MPEG1 L3 frame = 1152 samples/ch; minimp3 outputs interleaved stereo,
so MINIMP3_MAX_SAMPLES_PER_FRAME = 1152*2 = 2304 int16.
Double-buffer: first half played while second half is decoded, vice-versa.*/
#define FRAME_SAMPLES    MINIMP3_MAX_SAMPLES_PER_FRAME /* 2304 int16, stereo */
#define PCM_HALF_LEN     FRAME_SAMPLES                 /* 2304 int16, stereo */
#define PCM_BUF_LEN      (PCM_HALF_LEN * 2U)           /* 4608 int16 total   */
#define MP3_IN_BUF_SIZE  (2048U)                       /* input read buffer  */

#define MP3_PLAYER_ENTERN_CRITICAL()    // __disable_irq()
#define MP3_PLAYER_EXIT_CRITICAL()      // __enable_irq()
/* typedef ------------------------------------------------------------------*/
typedef enum
{
    FILL_NONE = 0,
    FILL_FIRST_HALF,
    FILL_SECOND_HALF,
} fill_req_t;
/* variables ----------------------------------------------------------------*/
static mp3dec_t            s_dec;
static mp3dec_frame_info_t s_frame_info;
static const mp3_src_t    *sp_src;
static uint32_t            s_offset;
static int16_t             s_dma_buf[PCM_BUF_LEN];
static uint8_t             s_in_buf[MP3_IN_BUF_SIZE];
static volatile fill_req_t s_fill_req = FILL_NONE;
static volatile uint8_t    s_running  = 0;
/* DMA 物理上是否在运行 */
static volatile uint8_t s_dma_running = 0;
/* codec 是否处于唤醒状态（非 PDN）*/
static volatile uint8_t s_codec_awake = 0;

/* Forward declarations — needed for s_out_cb initializer below */
static void mp3_isr_player_tx_half_cplt(void);
static void mp3_isr_player_tx_cplt(void);

static const audio_out_cb_cfg_t s_out_cb = {
    .pf_half_cplt = mp3_isr_player_tx_half_cplt,
    .pf_cplt      = mp3_isr_player_tx_cplt,
};
/* private  functions  ------------------------------------------------------*/

/* Decode MP3 frames into p_out until PCM_HALF_LEN int16 are filled.
  Loops across frames so MPEG2 (576 samples/frame) and MPEG1 (1152
  samples/frame) both fill the buffer completely.  Expands mono to interleaved
  stereo in-place. */
static void decode_half(int16_t *p_out)
{
    memset(p_out, 0, PCM_HALF_LEN * sizeof(int16_t));
    if(!s_running)
        return;

    uint32_t written = 0; /* int16 slots filled in p_out */

    while(written < PCM_HALF_LEN && s_running)
    {
        if(s_offset >= sp_src->total_size)
        {
            s_running = 0; /* EOS */
            return;
        }

        uint32_t remaining = sp_src->total_size - s_offset;
        uint32_t to_read   = (remaining < MP3_IN_BUF_SIZE) ? remaining
                                                           : MP3_IN_BUF_SIZE;
        uint32_t avail     = sp_src->pf_read(sp_src->p_ctx, s_offset, s_in_buf,
                                             to_read);
        if(avail == 0)
        {
            return;
        }

        int samples = mp3dec_decode_frame(&s_dec, s_in_buf, (int)avail,
                                          p_out + written, &s_frame_info);

        if(s_frame_info.frame_bytes > 0)
        {
            s_offset += (uint32_t)s_frame_info.frame_bytes;
        }
        else
        {
            s_offset++;
            continue;
        }

        if(samples <= 0)
        {
            continue;
        }

        /* int16 count written by the decoder */
        uint32_t int16_out = (uint32_t)samples *
                             (uint32_t)s_frame_info.channels;

        if(s_frame_info.channels == 1)
        {
            /* mono→stereo in-place (right-to-left traversal is safe) */
            int16_t *pf = p_out + written;
            for(int i = samples - 1; i >= 0; i--)
            {
                pf[i * 2 + 1] = pf[i];
                pf[i * 2]     = pf[i];
            }
            int16_out = (uint32_t)samples * 2U;
        }

        /* safety: don't exceed buffer */
        if(written + int16_out > PCM_HALF_LEN)
        {
            int16_out = PCM_HALF_LEN - written;
        }
        written += int16_out;
    }
}

/* ISR callbacks — set flag only, no heavy work here */
static void mp3_isr_player_tx_half_cplt(void)
{
    s_fill_req = FILL_FIRST_HALF;
}

static void mp3_isr_player_tx_cplt(void)
{
    s_fill_req = FILL_SECOND_HALF;
}
/* exported functions -------------------------------------------------------*/
/**
 * @brief            :  [mp3_player_init]
 */
void mp3_player_init(void)
{
    audio_out_init(&s_out_cb);
}
/**
 * @brief            :  [mp3_player_start]
 * @param[in]        :  [const mp3_src_t *p_src]
 */
void mp3_player_start(const mp3_src_t *p_src)
{
    mp3dec_init(&s_dec);
    sp_src        = p_src;
    s_offset      = 0;
    s_running     = 1;
    s_codec_awake = 1;
    MP3_PLAYER_ENTERN_CRITICAL();
    s_fill_req = FILL_NONE;
    MP3_PLAYER_EXIT_CRITICAL();

    if(!s_dma_running)
    {
        /* 首次启动：预填两半，设置采样率，然后启动 DMA + 唤醒 codec */
        decode_half(&s_dma_buf[0]);
        audio_out_set_sample_rate((uint32_t)s_frame_info.hz);
        decode_half(&s_dma_buf[PCM_HALF_LEN]);
        s_dma_running = 1;
        audio_out_start(s_dma_buf, PCM_BUF_LEN);
    }
    else
    {
        /* DMA 已在运行（soft_stop 后保持运行），只需唤醒 codec */
        audio_out_play();
    }
#ifdef MP3_PLARER_DBG
    log_i("start: sr=%d ch=%d rate=%d fram byte=%d dma_was_running=%d",
          s_frame_info.hz,s_frame_info.channels, s_frame_info.bitrate_kbps,
          s_frame_info.frame_bytes,s_dma_running);
#endif // end of MP3_PLARER_DBG
}
/**
 * @brief            :  [mp3_player_stop]
 */
void mp3_player_stop(void)
{
    s_running     = 0;
    s_codec_awake = 0;
    s_dma_running = 0;
    audio_out_stop();
#ifdef MP3_PLARER_DBG
    log_i("stopped at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG
}
/**
 * @brief            :  [mp3_player_soft_stop]
 */
void mp3_player_soft_stop(void)
{
    if(!s_codec_awake)
    {
        return; /* codec 已在 PDN，防止重复 I2C 写入 */
    }

    s_codec_awake = 0;
    s_running     = 0;
    MP3_PLAYER_ENTERN_CRITICAL();
    s_fill_req = FILL_NONE;
    MP3_PLAYER_EXIT_CRITICAL();
    audio_out_soft_stop(); /* 仅写 PDN，DMA 保持运行 */
#ifdef MP3_PLARER_DBG
    log_i("soft_stop at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG
}
/**
 * @brief            :  [mp3_player_is_playing]
 * @retval           :  [s_running]
 */
uint8_t mp3_player_is_playing(void)
{
    return s_running;
}
/**
 * @brief            :  [mp3_player_pause]
 */
void mp3_player_pause(void)
{
    s_running = 0;
#ifdef MP3_PLARER_DBG
    log_i("pause at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG
}
/**
 * @brief            :  [mp3_player_resume]
 */
void mp3_player_resume(void)
{
    s_running = 1;
#ifdef MP3_PLARER_DBG
    log_i("resume at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG
}

/* Call from main while(1). Decodes the next frame into whichever half
 * the ISR flagged. Must complete within one frame period (~26 ms). */
void mp3_player_process(void)
{
    MP3_PLAYER_ENTERN_CRITICAL();
    fill_req_t req = s_fill_req;
    s_fill_req     = FILL_NONE; /* atomic read-clear */
    MP3_PLAYER_EXIT_CRITICAL(); /* re-enable BEFORE the early return */
    int16_t *p_buff_source = NULL;
    if(req == FILL_NONE)
    {
        return;
    }

    if(req == FILL_FIRST_HALF)
    {
        p_buff_source = &s_dma_buf[0];
    }
    else
    {
        p_buff_source = &s_dma_buf[PCM_HALF_LEN];
    }
    decode_half(p_buff_source);

    if(!s_running)
    {
        /* Zero the OTHER half too — it still holds the last decoded frame.
         * DMA may be reading it right now; overwriting with silence is
         * safe and prevents that leftover PCM from being audible. */
        int16_t *p_other = (req == FILL_FIRST_HALF) ? &s_dma_buf[PCM_HALF_LEN]
                                                    : &s_dma_buf[0];
                                                    
        memset(p_other, 0, PCM_HALF_LEN * sizeof(int16_t));
        mp3_player_soft_stop();
    }
}

/* end of  file -------------------------------------------------------------*/
