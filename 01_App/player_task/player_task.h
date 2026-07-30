/**
 ******************************************************************************
 *@file    :   player_task.h
 *@brief   :   playerTask — SD mount, file management, playback control.
 *             Receives control commands from uart_task via command queue.
 *@version :   V2.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef PLAYER_TASK_H
#define PLAYER_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    PLAYER_CMD_PLAY,
    PLAYER_CMD_START,
    PLAYER_CMD_STOP,
    PLAYER_CMD_RESUME,
    PLAYER_CMD_NEXT,
} player_cmd_t;

void player_task_create(void);
void player_task_send_cmd(player_cmd_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_TASK_H */
