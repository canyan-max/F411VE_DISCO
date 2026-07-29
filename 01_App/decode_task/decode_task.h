/**
 ******************************************************************************
 *@file    :   decode_task.h
 *@brief   :   decodeTask — MP3 decode + DMA fill scheduling task.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef DECODE_TASK_H
#define DECODE_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

void decode_task_init(void);
void decode_task_create(void);
void decode_task_signal(void);

#ifdef __cplusplus
}
#endif

#endif /* DECODE_TASK_H */
