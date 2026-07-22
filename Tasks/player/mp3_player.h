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
#include <stdint.h>   /* stdint lib header file. */
#include <stddef.h>   /* stdint lib header file. */
#include "FreeRTOS.h"
#include "task.h"
/* define -------------------------------------------------------------------*/

/* typedef ------------------------------------------------------------------*/
/* Data source abstraction — flash array or future SD card both implement
 * the same pf_read interface; mp3_player has no knowledge of the source. */
typedef struct
{
    /* Read len bytes from the source at offset into p_buf.
     * Returns the number of bytes actually read.
     * Caller guarantees offset + len <= total_size. */
    uint32_t (*pf_read)(void    *p_ctx,
                        uint32_t offset,
                        uint8_t *p_buf,
                        uint32_t len);
    void    *p_ctx;
    uint32_t total_size;
} mp3_src_t;

/* exported types -----------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/

/* functions ----------------------------------------------------------------*/
/* Register callbacks with audio_out and initialise the codec.
 * Must be called once before mp3_player_start. */
void    mp3_player_init(void);
void    mp3_player_bind_task(TaskHandle_t handle);
void    mp3_player_start(const mp3_src_t *p_src);
void    mp3_player_stop(void);
void    mp3_player_soft_stop(void);
void    mp3_player_pause(void);
void    mp3_player_resume(void);
void    mp3_player_process(void);
uint8_t mp3_player_is_playing(void);

#ifdef __cplusplus
}
#endif

#endif /* MP3_PLAYER_H */
