/**
 ******************************************************************************
 *@file               :   mp3_player.h
 *@brief              :   Provide the HAL APIs of description.
 *@version            :   V1.0
 *@note               :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef MP3_PLAYER_H
#define MP3_PLAYER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include "media_src.h"

/* functions ----------------------------------------------------------------*/
void    mp3_player_start(const media_src_t *p_src);
void    mp3_player_stop(void);
void    mp3_player_soft_stop(void);
void    mp3_player_pause(void);
void    mp3_player_resume(void);
void    mp3_player_process(void);
uint8_t mp3_player_is_playing(void);

/* ISR-safe state update — call from DMA half/complete callbacks */
void    mp3_player_on_half_cplt(void);
void    mp3_player_on_cplt(void);

#ifdef __cplusplus
}
#endif

#endif /* MP3_PLAYER_H */
