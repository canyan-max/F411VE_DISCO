#define MP3P_TAG "mp3p"
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_ONLY_MP3     1
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "mp3_player.h"
#include "audio_out.h"
#include "stm32f4xx.h" /* __disable_irq / __enable_irq (CMSIS) */
#include <string.h>

#define MP3_PLARER_DBG
#ifdef MP3_PLARER_DBG
#define MP3_PLARERTAG "mp3player"
#include "elog.h"
#endif // end of MP3_PLARER_DBG

/* Each MPEG1 L3 frame = 1152 samples/ch; minimp3 outputs interleaved stereo,
 * so MINIMP3_MAX_SAMPLES_PER_FRAME = 1152*2 = 2304 int16.
 * Double-buffer: first half played while second half is decoded, vice-versa. */
#define FRAME_SAMPLES    MINIMP3_MAX_SAMPLES_PER_FRAME /* 2304 int16, stereo */
#define PCM_HALF_LEN     FRAME_SAMPLES
#define PCM_BUF_LEN      (PCM_HALF_LEN * 2U)           /* 4608 int16 total   */
#define MP3_IN_BUF_SIZE  (2048U)                       /* input read buffer  */

typedef enum
{
    FILL_NONE = 0,
    FILL_FIRST_HALF,
    FILL_SECOND_HALF,
} fill_req_t;

static mp3dec_t            s_dec;
static mp3dec_frame_info_t s_frame_info;
static const mp3_src_t    *sp_src;
static uint32_t            s_offset;
static int16_t             s_dma_buf[PCM_BUF_LEN];
static uint8_t             s_in_buf[MP3_IN_BUF_SIZE];
static volatile fill_req_t s_fill_req    = FILL_NONE;
static volatile uint8_t    s_running     = 0;
static volatile uint8_t    s_dma_running = 0; /* DMA 物理上是否在运行 */
static volatile uint8_t    s_codec_awake = 0; /* codec 是否处于唤醒状态（非 PDN）*/

/* Forward declarations — needed for s_out_cb initializer below */
static void mp3_isr_player_tx_half_cplt(void);
static void mp3_isr_player_tx_cplt(void);

static const audio_out_cb_cfg_t s_out_cb = {
    .pf_half_cplt = mp3_isr_player_tx_half_cplt,
    .pf_cplt      = mp3_isr_player_tx_cplt,
};

void mp3_player_init(void)
{
    audio_out_init(&s_out_cb);
}

/* Decode one MP3 frame into p_out (stereo interleaved, PCM_HALF_LEN int16).
 * Reads from sp_src via pf_read; handles mono→stereo expansion in-place.
 * Zeroes p_out on error or end-of-stream (produces silence instead of noise). */
static void decode_frame(int16_t *p_out)
{
    memset(p_out, 0, PCM_HALF_LEN * sizeof(int16_t));
    if(!s_running)
        return;

    if(s_offset >= sp_src->total_size)
    {
        s_running = 0; /* EOS — silence this half; process() will stop DMA */
        return;
    }

    uint32_t remaining = sp_src->total_size - s_offset;
    uint32_t to_read   = (remaining < MP3_IN_BUF_SIZE) ? remaining
                                                        : MP3_IN_BUF_SIZE;
    uint32_t avail     = sp_src->pf_read(sp_src->p_ctx, s_offset,
                                          s_in_buf, to_read);
    if(avail == 0)
        return;

    int samples = mp3dec_decode_frame(&s_dec, s_in_buf, (int)avail,
                                      p_out, &s_frame_info);

    if(s_frame_info.frame_bytes > 0)
        s_offset += (uint32_t)s_frame_info.frame_bytes;
    else
        s_offset++; /* skip 1 byte on sync error to avoid infinite loop */

    if(samples <= 0)
        return;

    if(s_frame_info.channels == 1)
    {
        /* expand mono to stereo in-place (right-to-left traversal is safe) */
        for(int i = samples - 1; i >= 0; i--)
        {
            p_out[i * 2 + 1] = p_out[i];
            p_out[i * 2]     = p_out[i];
        }
    }
}

void mp3_player_start(const mp3_src_t *p_src)
{
    mp3dec_init(&s_dec);
    sp_src      = p_src;
    s_offset    = 0;
    s_running   = 1;
    s_codec_awake = 1;
    __disable_irq();
    s_fill_req = FILL_NONE;
    __enable_irq();

    if(!s_dma_running)
    {
        /* 首次启动：预填两帧，然后启动 DMA + 唤醒 codec */
        decode_frame(&s_dma_buf[0]);
        decode_frame(&s_dma_buf[PCM_HALF_LEN]);
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
          s_frame_info.hz, s_frame_info.channels, s_frame_info.bitrate_kbps,
          s_frame_info.frame_bytes,s_dma_running);
#endif // end of MP3_PLARER_DBG
}

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

void mp3_player_soft_stop(void)
{
    if(!s_codec_awake)
        return; /* codec 已在 PDN，防止重复 I2C 写入 */
    s_codec_awake = 0;
    s_running     = 0;
    __disable_irq();
    s_fill_req = FILL_NONE;
    __enable_irq();
    audio_out_soft_stop(); /* 仅写 PDN，DMA 保持运行 */
#ifdef MP3_PLARER_DBG
    log_i("soft_stop at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG
}

uint8_t mp3_player_is_playing(void)
{
    return s_running;
}

void mp3_player_pause(void)
{
    s_running = 0;
    /* DMA 继续运行，decode_frame() 在 s_running=0 时填零，
     * codec 收到全零 PCM → DAC 静音 → 不碰硬件 MUTE/POWER 寄存器，无白噪声 */
#ifdef MP3_PLARER_DBG
    log_i("pause at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG

}

void mp3_player_resume(void)
{
    s_running = 1;
    /* DMA 本来就在跑，下一次 ISR 触发后 decode_frame() 恢复解码真实音频 */
#ifdef MP3_PLARER_DBG
    log_i("resume at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG

}

/* Call from main while(1). Decodes the next frame into whichever half
 * the ISR flagged. Must complete within one frame period (~26 ms). */
void mp3_player_process(void)
{
    __disable_irq();
    fill_req_t req = s_fill_req;
    s_fill_req     = FILL_NONE; /* atomic read-clear */
    __enable_irq();             /* re-enable BEFORE the early return */
    int16_t             *p_buff_source = NULL;
    
    if(req == FILL_NONE)
        return;
    if(req == FILL_FIRST_HALF)
    {
        p_buff_source = &s_dma_buf[0];
    }
    else 
    {
        p_buff_source = &s_dma_buf[PCM_HALF_LEN];
    }
    decode_frame(p_buff_source);

    if(!s_running)
    {
        /* Zero the OTHER half too — it still holds the last decoded frame.
         * DMA may be reading it right now; overwriting with silence is
         * safe and prevents that leftover PCM from being audible. */
        int16_t *p_other = (req == FILL_FIRST_HALF)
                               ? &s_dma_buf[PCM_HALF_LEN]
                               : &s_dma_buf[0];
        memset(p_other, 0, PCM_HALF_LEN * sizeof(int16_t));
        mp3_player_soft_stop();
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
