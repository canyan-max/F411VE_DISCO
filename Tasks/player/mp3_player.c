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
static volatile fill_req_t s_fill_req = FILL_NONE;
static volatile uint8_t    s_running  = 0;

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
    sp_src     = p_src;
    s_offset   = 0;
    s_fill_req = FILL_NONE;
    s_running  = 1;

    decode_frame(&s_dma_buf[0]);
    decode_frame(&s_dma_buf[PCM_HALF_LEN]);

    audio_out_start(s_dma_buf, PCM_BUF_LEN);
#ifdef MP3_PLARER_DBG
    log_i("start: sr=%d ch=%d", s_frame_info.hz, s_frame_info.channels);
#endif // end of MP3_PLARER_DBG

}

void mp3_player_stop(void)
{
    s_running = 0;
    audio_out_stop();
#ifdef MP3_PLARER_DBG
    log_i("stopped at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG

}

void mp3_player_pause(void)
{
    s_running = 0;
    audio_out_pause();
#ifdef MP3_PLARER_DBG
    log_i("pause at offset=%lu", s_offset);
#endif // end of MP3_PLARER_DBG

}

void mp3_player_resume(void)
{
    s_running = 1;
    audio_out_resume();
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
        mp3_player_pause();
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
