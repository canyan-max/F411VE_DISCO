/**
 ******************************************************************************
 *@file    :   player_task.c
 *@brief   :   playerTask — SD mount, MP3 file traversal, playback control.
 *             Signals decodeTask to start decoding. Future: responds to
 *             play/pause/next commands from uart_task.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "fatfs.h"
#include "media_src.h"
#include "mp3_player.h"
#include "sd_manager.h"
#include "decode_task.h"
#include "player_task.h"
#ifdef USER_DEBUG_LOG
#include "elog.h"
#endif

/* private variables --------------------------------------------------------*/
static const osThreadAttr_t s_attr = {
    .name       = "playerTask",
    .stack_size = 1024U * 4U,
    .priority   = (osPriority_t)osPriorityNormal,
};

/* private functions --------------------------------------------------------*/
static uint8_t find_next_mp3(const FILINFO *p_files, uint8_t count,
                              uint8_t start)
{
    if (count == 0)
    {
        return 0xFF;
    }
    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t     idx = (uint8_t)((start + i) % count);
        size_t      len = strlen(p_files[idx].fname);
        if (len < 4)
        {
            continue;
        }
        const char *ext = p_files[idx].fname + len - 4;
        if (ext[0] == '.' &&
            (ext[1] == 'M' || ext[1] == 'm') &&
            (ext[2] == 'P' || ext[2] == 'p') &&
             ext[3] == '3')
        {
            return idx;
        }
    }
    return 0xFF;
}

static void StartPlayerTask(void *argument)
{
    static media_src_t s_src;
    static FILINFO     s_files[16];
    static char        s_path[32];
    static char        s_log_buf[64];
    static uint32_t    s_play_count = 0;
    static uint16_t    s_song_counts[16];
    uint8_t s_file_count = 0;
    uint8_t s_play_idx   = 0xFF;
    (void)argument;

    if (sd_manager_mount(USERPath) != PLATFORM_ERR_OK)
    {
#ifdef USER_DEBUG_LOG
        log_e("[player] sd mount failed");
#endif
        for (;;)
        {
            osDelay(1000);
        }
    }

    sd_manager_list_dir(USERPath, s_files, 16, &s_file_count);
#ifdef USER_DEBUG_LOG
    log_i("[player] mount ok, %d files", s_file_count);
    for (uint8_t i = 0; i < s_file_count; i++)
    {
        log_i("[player] %s  %lu B", s_files[i].fname, s_files[i].fsize);
    }
#endif

    s_play_idx = find_next_mp3(s_files, s_file_count, 0);
    if (s_play_idx != 0xFF)
    {
        snprintf(s_path, sizeof(s_path), "0:/%s", s_files[s_play_idx].fname);
        if (sd_manager_open(s_path) == PLATFORM_ERR_OK)
        {
            sd_manager_get_src(&s_src);
            mp3_player_start(&s_src);
            decode_task_signal();
#ifdef USER_DEBUG_LOG
            log_i("[player] playing %s", s_files[s_play_idx].fname);
#endif
        }
    }

    for (;;)
    {
        if (s_play_idx != 0xFF && !mp3_player_is_playing())
        {
            sd_manager_close();
            s_play_count++;
            s_song_counts[s_play_idx]++;
            int log_len = snprintf(s_log_buf, sizeof(s_log_buf),
                                   "[%u] %s %lu B (x%u)\r\n",
                                   s_play_count,
                                   s_files[s_play_idx].fname,
                                   s_files[s_play_idx].fsize,
                                   s_song_counts[s_play_idx]);
            if (log_len > 0 &&
                sd_manager_open_write("0:/mp3log.txt") == PLATFORM_ERR_OK)
            {
                sd_manager_write((const uint8_t *)s_log_buf,
                                 (uint32_t)log_len, NULL);
                sd_manager_close_write();
            }
            uint8_t next = find_next_mp3(s_files, s_file_count,
                                         (uint8_t)((s_play_idx + 1U) %
                                                    s_file_count));
            if (next != 0xFF)
            {
                s_play_idx = next;
                snprintf(s_path, sizeof(s_path), "0:/%s",
                         s_files[s_play_idx].fname);
                if (sd_manager_open(s_path) == PLATFORM_ERR_OK)
                {
                    sd_manager_get_src(&s_src);
                    mp3_player_start(&s_src);
                    decode_task_signal();
#ifdef USER_DEBUG_LOG
                    log_i("[player] playing %s", s_files[s_play_idx].fname);
#endif
                }
            }
        }
        osDelay(500);
    }
}

/* exported functions -------------------------------------------------------*/
void player_task_create(void)
{
    osThreadNew(StartPlayerTask, NULL, &s_attr);
}

/* end of file --------------------------------------------------------------*/
