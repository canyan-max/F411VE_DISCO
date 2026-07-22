/**
 ******************************************************************************
 *@file               :   mp3_player.c
 *@brief              :   Provide the HAL APIs of description.
 *@version            :   V1.0
 *@note               :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <string.h>
#include <stddef.h>
#include "mp3_player.h"
#include "audio_out.h"


#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif

// #define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_ONLY_MP3     1
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"

/* define   -----------------------------------------------------------------*/
#define FRAME_SAMPLES        MINIMP3_MAX_SAMPLES_PER_FRAME
#define PCM_HALF_LEN         FRAME_SAMPLES
#define PCM_BUF_LEN          (PCM_HALF_LEN * 2U)
#define MP3_IN_BUF_SIZE      (2048U)
#define MP3_MAX_FRAME_BYTES  (1440U)

#define MP3_PLAYER_ENTERN_CRITICAL()
#define MP3_PLAYER_EXIT_CRITICAL()

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
static const media_src_t    *sp_src;
static uint32_t            s_offset;
static uint32_t            s_in_fill;
static int16_t             s_dma_buf[PCM_BUF_LEN];
static uint8_t             s_in_buf[MP3_IN_BUF_SIZE];
static volatile fill_req_t s_fill_req   = FILL_NONE;
static volatile uint8_t    s_running    = 0;
static volatile uint8_t    s_dma_running = 0;
static volatile uint8_t    s_codec_awake = 0;
static volatile uint8_t    s_start_req  = 0;
/* private functions --------------------------------------------------------*/
static void refill_input(void)
{
    if (s_in_fill >= MP3_MAX_FRAME_BYTES)
    {
        return;
    }
    uint32_t next = s_offset + s_in_fill;
    if (next >= sp_src->total_size)
    {
        return;
    }
    uint32_t space   = MP3_IN_BUF_SIZE - s_in_fill;
    uint32_t remain  = sp_src->total_size - next;
    uint32_t to_read = (remain < space) ? remain : space;
    uint32_t got     = sp_src->pf_read(sp_src->p_ctx, next,
                                       s_in_buf + s_in_fill, to_read);
    s_in_fill += got;
}

static void consume_input(uint32_t bytes)
{
    s_in_fill -= bytes;
    if (s_in_fill > 0)
    {
        memmove(s_in_buf, s_in_buf + bytes, s_in_fill);
    }
    s_offset += bytes;
}

static uint32_t mono_to_stereo(int16_t *p_buf, int samples)
{
    for (int i = samples - 1; i >= 0; i--)
    {
        p_buf[i * 2 + 1] = p_buf[i];
        p_buf[i * 2]     = p_buf[i];
    }
    return (uint32_t)samples * 2U;
}

static void decode_half(int16_t *p_out)
{
    memset(p_out, 0, PCM_HALF_LEN * sizeof(int16_t));
    if (!s_running)
    {
        return;
    }

    uint32_t written = 0;
    while (written < PCM_HALF_LEN && s_running)
    {
        refill_input();

        if (s_in_fill == 0)
        {
            s_running = 0;
            return;
        }

        int samples = mp3dec_decode_frame(&s_dec, s_in_buf, (int)s_in_fill,
                                          p_out + written, &s_frame_info);

        uint32_t consumed = (s_frame_info.frame_bytes > 0)
                          ? (uint32_t)s_frame_info.frame_bytes
                          : 1U;
        consume_input(consumed);

        if (samples <= 0)
        {
            continue;
        }

        uint32_t out_count = (s_frame_info.channels == 1)
                           ? mono_to_stereo(p_out + written, samples)
                           : (uint32_t)samples * 2U;

        if (written + out_count > PCM_HALF_LEN)
        {
            out_count = PCM_HALF_LEN - written;
        }
        written += out_count;
    }
}

/* exported functions -------------------------------------------------------*/
void mp3_player_on_half_cplt(void)
{
    s_fill_req = FILL_FIRST_HALF;
}

void mp3_player_on_cplt(void)
{
    s_fill_req = FILL_SECOND_HALF;
}

void mp3_player_start(const media_src_t *p_src)
{
    sp_src      = p_src;
    s_start_req = 1;
}

void mp3_player_stop(void)
{
    s_running     = 0;
    s_codec_awake = 0;
    s_dma_running = 0;
    audio_out_stop();
#ifdef USER_DEBUG_LOG
    log_i("stopped at offset=%lu", s_offset);
#endif
}

void mp3_player_soft_stop(void)
{
    if (!s_codec_awake)
    {
        return;
    }
    s_codec_awake = 0;
    s_running     = 0;
    MP3_PLAYER_ENTERN_CRITICAL();
    s_fill_req = FILL_NONE;
    MP3_PLAYER_EXIT_CRITICAL();
    audio_out_soft_stop();
#ifdef USER_DEBUG_LOG
    log_i("soft_stop at offset=%lu", s_offset);
#endif
}

uint8_t mp3_player_is_playing(void)
{
    return s_running;
}

void mp3_player_pause(void)
{
    s_running = 0;
#ifdef USER_DEBUG_LOG
    log_i("pause at offset=%lu", s_offset);
#endif
}

void mp3_player_resume(void)
{
    s_running = 1;
#ifdef USER_DEBUG_LOG
    log_i("resume at offset=%lu", s_offset);
#endif
}

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
#ifdef USER_DEBUG_LOG
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

    if (req == FILL_NONE)
    {
        return;
    }

    int16_t *p_buf = (req == FILL_FIRST_HALF) ? &s_dma_buf[0]
                                               : &s_dma_buf[PCM_HALF_LEN];
    decode_half(p_buf);

    if (!s_running)
    {
//        int16_t *p_other = (req == FILL_FIRST_HALF) ? &s_dma_buf[PCM_HALF_LEN]
//                                                    : &s_dma_buf[0];

//        memset(p_other, 0, PCM_HALF_LEN * sizeof(int16_t));
        mp3_player_soft_stop();
    }
}

/* end of file --------------------------------------------------------------*/
