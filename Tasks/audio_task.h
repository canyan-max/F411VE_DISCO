/**
 ******************************************************************************
 *@file    :   audio_task.h
 *@brief   :   audioTask — MP3 decode scheduling task.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

void audio_task_create(void);
void audio_task_signal(void);
void audio_task_init(void);
#ifdef __cplusplus
}
#endif

#endif /* AUDIO_TASK_H */
