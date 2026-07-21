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
#include <string.h>     /* string lib header file. */
#include <stddef.h>     /* stdint lib header file. */
#include "mp3_player.h" /* mp3_player lib header file. */
#include "audio_out.h"  /* audio_out lib header file. */
#ifdef MP3_PLARER_DBG
#include "elog.h"       /* elog lib header file. */
#endif // end of MP3_PLARER_DBG
// #define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_ONLY_MP3     1
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"    /* minimp3 lib header file. */
/* define   -----------------------------------------------------------------*/
/* Each MPEG1 L3 frame = 1152 samples/ch; minimp3 outputs interleaved stereo,
so MINIMP3_MAX_SAMPLES_PER_FRAME = 1152*2 = 2304 int16.
Double-buffer: first half played while second half is decoded, vice-versa.*/
#define FRAME_SAMPLES    MINIMP3_MAX_SAMPLES_PER_FRAME // 2304 int16, stereo
#define PCM_HALF_LEN     FRAME_SAMPLES                 // 2304 int16, stereo
#define PCM_BUF_LEN      (PCM_HALF_LEN * 2U)           // 4608 int16 total  
#define MP3_IN_BUF_SIZE  (2048U)                       // input read buffer 

/* MP3 最大帧字节数（MPEG1 L3 320kbps 约 1045B，取安全上限） */
#define MP3_MAX_FRAME_BYTES  (1440U)  // mp3 max byte
#define MP3_PLAYER_ENTERN_CRITICAL()  // __disable_irq()
#define MP3_PLAYER_EXIT_CRITICAL()    // __enable_irq()
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
static uint32_t            s_offset;  /* s_in_buf[0] 对应的文件绝对偏移 */
static uint32_t            s_in_fill; /* s_in_buf 中有效字节数 */
static int16_t             s_dma_buf[PCM_BUF_LEN];
static uint8_t             s_in_buf[MP3_IN_BUF_SIZE];
static volatile fill_req_t s_fill_req = FILL_NONE;
static volatile uint8_t    s_running  = 0;
/* DMA 物理上是否在运行 */
static volatile uint8_t s_dma_running = 0;
/* codec 是否处于唤醒状态（非 PDN）*/
static volatile uint8_t s_codec_awake = 0;
static TaskHandle_t     s_audio_task  = NULL;
static volatile uint8_t s_start_req   = 0;

/* Forward declarations — needed for s_out_cb initializer below */
static void mp3_isr_player_tx_half_cplt(void);
static void mp3_isr_player_tx_cplt(void);

static const audio_out_cb_cfg_t s_out_cb = {
    .pf_half_cplt = mp3_isr_player_tx_half_cplt,
    .pf_cplt      = mp3_isr_player_tx_cplt,
};
/* private  functions  ------------------------------------------------------*/

/* Append data from the source into s_in_buf until it holds at least one frame
 */
static void refill_input(void)
{
    if(s_in_fill >= MP3_MAX_FRAME_BYTES)
    {
        return;
    }
    uint32_t next = s_offset + s_in_fill; /* next unread file byte */
    if(next >= sp_src->total_size)
    {
        return;
    }
    uint32_t space   = MP3_IN_BUF_SIZE - s_in_fill;
    uint32_t remain  = sp_src->total_size - next;
    uint32_t to_read = (remain < space) ? remain : space;
    uint32_t got = sp_src->pf_read(sp_src->p_ctx, next, s_in_buf + s_in_fill,
                                   to_read);
    s_in_fill += got;
}

/* Discard `bytes` from the front of s_in_buf and slide remaining data down */
static void consume_input(uint32_t bytes)
{
    s_in_fill -= bytes;
    if(s_in_fill > 0)
    {
        memmove(s_in_buf, s_in_buf + bytes, s_in_fill);
    }
    s_offset += bytes;
}

/* Expand mono samples to interleaved stereo in-place; returns new int16 count
 */
static uint32_t mono_to_stereo(int16_t *p_buf, int samples)
{
    for(int i = samples - 1; i >= 0; i--)
    {
        p_buf[i * 2 + 1] = p_buf[i];
        p_buf[i * 2]     = p_buf[i];
    }
    return (uint32_t)samples * 2U;
}

static void decode_half(int16_t *p_out)
{
    memset(p_out, 0, PCM_HALF_LEN * sizeof(int16_t));
    if(!s_running)
    {
        return;
    }

    uint32_t written = 0;
    while(written < PCM_HALF_LEN && s_running)
    {
        refill_input();

        if(s_in_fill == 0)
        {
            s_running = 0; /* EOS */
            return;
        }

        int      samples = mp3dec_decode_frame(&s_dec, s_in_buf, (int)s_in_fill,
                                               p_out + written, &s_frame_info);
        uint32_t consumed=1U;
        if(s_frame_info.frame_bytes > 0)
        {
            consumed = (uint32_t)s_frame_info.frame_bytes;
        }
        else
        {
            consumed = 1U;
        }
        consume_input(consumed);

        if(samples <= 0)
        {
            continue; /* non-audio frame continue next frame */
        }

        uint32_t out_count;
        if(s_frame_info.channels == 1)
        {
            out_count = mono_to_stereo(p_out + written, samples);
        }
        else
        {
            out_count = (uint32_t)samples * 2U;
        }
        
        if(written + out_count > PCM_HALF_LEN)
        {
            out_count = PCM_HALF_LEN - written;
        }
        written += out_count;
    }
}

static void mp3_isr_player_tx_half_cplt(void)
{
    s_fill_req = FILL_FIRST_HALF;
    if (s_audio_task != NULL)
    {
        BaseType_t woken = pdFALSE;
        vTaskNotifyGiveFromISR(s_audio_task, &woken);
        portYIELD_FROM_ISR(woken);
    }
}

static void mp3_isr_player_tx_cplt(void)
{
    s_fill_req = FILL_SECOND_HALF;
    if (s_audio_task != NULL)
    {
        BaseType_t woken = pdFALSE;
        vTaskNotifyGiveFromISR(s_audio_task, &woken);
        portYIELD_FROM_ISR(woken);
    }
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
 * @brief            :  [mp3_player_bind_task]
 */
void mp3_player_bind_task(TaskHandle_t handle)
{
    s_audio_task = handle;
}
/**
 * @brief            :  [mp3_player_start]
 * @param[in]        :  [const mp3_src_t *p_src]
 */
void mp3_player_start(const mp3_src_t *p_src)
{
    sp_src      = p_src;
    s_start_req = 1;
    if (s_audio_task != NULL)
    {
        xTaskNotifyGive(s_audio_task);
    }
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

/* Call from audioTask. Decodes the next frame into whichever half
 * the ISR flagged. Must complete within one frame period (~48 ms). */
void mp3_player_process(void)
{
    if (s_start_req)
    {
        s_start_req   = 0;
        mp3dec_init(&s_dec);
        s_offset      = 0;
        s_in_fill     = 0;
        s_running     = 1;
        s_codec_awake = 1;
        s_fill_req    = FILL_NONE;

        decode_half(&s_dma_buf[0]);
        audio_out_set_sample_rate((uint32_t)s_frame_info.hz);
        decode_half(&s_dma_buf[PCM_HALF_LEN]);

        s_dma_running = 1;
        audio_out_start(s_dma_buf, PCM_BUF_LEN);
//        s_fill_req    = FILL_NONE;
        
#ifdef MP3_PLARER_DBG
        log_i("start: sr=%d ch=%d rate=%d frame_bytes=%d",
              s_frame_info.hz, s_frame_info.channels,
              s_frame_info.bitrate_kbps, s_frame_info.frame_bytes);
#endif
        return;
    }
    
    MP3_PLAYER_ENTERN_CRITICAL();
    fill_req_t req = s_fill_req;
    s_fill_req     = FILL_NONE;
    MP3_PLAYER_EXIT_CRITICAL();
    
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
