/**
 ******************************************************************************
 *@file    :   player_task.c
 *@brief   :   playerTask — SD mount, MP3 file traversal, playback control.
 *             Receives player_cmd_t from uart_task via osMessageQueue.
 *             State machine: IDLE / PLAYING / PAUSED.
 *@version :   V2.0
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

/* private typedef ----------------------------------------------------------*/
typedef enum
{
    PLAYER_ST_IDLE,
    PLAYER_ST_PLAYING,
    PLAYER_ST_PAUSED,
} player_st_t;

/* private variables --------------------------------------------------------*/
static osMessageQueueId_t s_cmd_queue;
static player_st_t        s_player_st  = PLAYER_ST_IDLE;
static media_src_t        s_src;
static FILINFO            s_files[16];
static char               s_path[32];
static uint8_t            s_file_count = 0U;
static uint8_t            s_play_idx   = 0xFFU;

static const osThreadAttr_t s_attr = {
    .name       = "playerTask",
    .stack_size = 1024U * 4U,
    .priority   = (osPriority_t)osPriorityNormal,
};

/* private functions --------------------------------------------------------*/
static uint8_t find_next_mp3(const FILINFO *p_files, uint8_t count,
                              uint8_t start)
{
    if (count == 0U)
    {
        return 0xFFU;
    }
    for (uint8_t i = 0U; i < count; i++)
    {
        uint8_t     idx = (uint8_t)((start + i) % count);
        size_t      len = strlen(p_files[idx].fname);
        if (len < 4U)
        {
            continue;
        }
        const char *p_ext = p_files[idx].fname + len - 4U;
        if (p_ext[0] == '.' &&
            (p_ext[1] == 'M' || p_ext[1] == 'm') &&
            (p_ext[2] == 'P' || p_ext[2] == 'p') &&
             p_ext[3] == '3')
        {
            return idx;
        }
    }
    return 0xFFU;
}

static void do_play_idx(uint8_t idx)
{
    snprintf(s_path, sizeof(s_path), "0:/%s", s_files[idx].fname);
    if (sd_manager_open(s_path) != PLATFORM_ERR_OK)
    {
        return;
    }
    sd_manager_get_src(&s_src);
    s_play_idx  = idx;
    s_player_st = PLAYER_ST_PLAYING;
    mp3_player_start(&s_src);
    decode_task_signal();
#ifdef USER_DEBUG_LOG
    log_i("[player] playing %s", s_files[idx].fname);
#endif
}

static void do_start(void)
{
    if (s_play_idx != 0xFFU)
    {
        mp3_player_soft_stop();
        sd_manager_close();
        s_play_idx = 0xFFU;
    }
    uint8_t idx = find_next_mp3(s_files, s_file_count, 0U);
    if (idx != 0xFFU)
    {
        do_play_idx(idx);
    }
    else
    {
        s_player_st = PLAYER_ST_IDLE;
    }
}

static void do_next(void)
{
    sd_manager_close();
    if (s_file_count == 0U)
    {
        s_play_idx  = 0xFFU;
        s_player_st = PLAYER_ST_IDLE;
        return;
    }
    uint8_t next_start = (uint8_t)((s_play_idx + 1U) % s_file_count);
    uint8_t idx        = find_next_mp3(s_files, s_file_count, next_start);
    if (idx != 0xFFU)
    {
        do_play_idx(idx);
    }
    else
    {
        s_play_idx  = 0xFFU;
        s_player_st = PLAYER_ST_IDLE;
    }
}

static void do_cmd(player_cmd_t cmd)
{
    switch (cmd)
    {
        case PLAYER_CMD_PLAY:
            if (s_player_st == PLAYER_ST_PLAYING)
            {
                mp3_player_pause();
                s_player_st = PLAYER_ST_PAUSED;
            }
            else if (s_player_st == PLAYER_ST_PAUSED)
            {
                mp3_player_resume();
                s_player_st = PLAYER_ST_PLAYING;
            }
            else
            {
                do_start();
            }
            break;

        case PLAYER_CMD_START:
            do_start();
            break;

        case PLAYER_CMD_STOP:
            if (s_player_st != PLAYER_ST_IDLE)
            {
                mp3_player_soft_stop();
                sd_manager_close();
                s_play_idx  = 0xFFU;
                s_player_st = PLAYER_ST_IDLE;
            }
            break;

        case PLAYER_CMD_RESUME:
            if (s_player_st == PLAYER_ST_PAUSED)
            {
                mp3_player_resume();
                s_player_st = PLAYER_ST_PLAYING;
            }
            break;

        case PLAYER_CMD_NEXT:
            if (s_player_st != PLAYER_ST_IDLE)
            {
                mp3_player_soft_stop();
                do_next();
            }
            break;

        default:
            break;
    }
}

static void StartPlayerTask(void *argument)
{
    static char     s_log_buf[64];
    static uint32_t s_play_count = 0U;
    static uint16_t s_song_counts[16];
    (void)argument;

    if (sd_manager_mount(USERPath) != PLATFORM_ERR_OK)
    {
#ifdef USER_DEBUG_LOG
        log_e("[player] sd mount failed");
#endif
        for (;;)
        {
            osDelay(1000U);
        }
    }

    sd_manager_list_dir(USERPath, s_files, 16U, &s_file_count);
#ifdef USER_DEBUG_LOG
    log_i("[player] mount ok, %d files", s_file_count);
    for (uint8_t i = 0U; i < s_file_count; i++)
    {
        log_i("[player] %s  %lu B", s_files[i].fname, s_files[i].fsize);
    }
#endif

    for (;;)
    {
        player_cmd_t cmd;
        if (osMessageQueueGet(s_cmd_queue, &cmd, NULL, 500U) == osOK)
        {
            do_cmd(cmd);
        }

        if (s_player_st == PLAYER_ST_PLAYING && !mp3_player_is_playing())
        {
            s_play_count++;
            s_song_counts[s_play_idx]++;
            int log_len = snprintf(s_log_buf, sizeof(s_log_buf),
                                   "[%u] %s %u B (x%u)\r\n",
                                   s_play_count,
                                   s_files[s_play_idx].fname,
                                   (uint32_t)s_files[s_play_idx].fsize,
                                   s_song_counts[s_play_idx]);
            if (log_len > 0 &&
                sd_manager_open_write("0:/mp3log.txt") == PLATFORM_ERR_OK)
            {
                sd_manager_write((const uint8_t *)s_log_buf,
                                 (uint32_t)log_len, NULL);
                sd_manager_close_write();
            }
            do_next();
        }
    }
}

/* exported functions -------------------------------------------------------*/
void player_task_create(void)
{
    s_cmd_queue = osMessageQueueNew(4U, sizeof(player_cmd_t), NULL);
    osThreadNew(StartPlayerTask, NULL, &s_attr);
}

void player_task_send_cmd(player_cmd_t cmd)
{
    osMessageQueuePut(s_cmd_queue, &cmd, 0U, 0U);
}

/* end of file --------------------------------------------------------------*/
