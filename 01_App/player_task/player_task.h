/**
 ******************************************************************************
 *@file    :   player_task.h
 *@brief   :   playerTask — SD mount, file management, playback control.
 *             Receives control commands (future: from uart_task).
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef PLAYER_TASK_H
#define PLAYER_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

void player_task_create(void);

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_TASK_H */
