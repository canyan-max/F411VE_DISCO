#ifndef MP3_PLAYER_H
#define MP3_PLAYER_H

#include <stdint.h>

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

/* Register callbacks with audio_out and initialise the codec.
 * Must be called once before mp3_player_start. */
void mp3_player_init(void);

void mp3_player_start(const mp3_src_t *p_src);
void mp3_player_stop(void);
void mp3_player_pause(void);
void mp3_player_resume(void);
void mp3_player_process(void);

#endif /* MP3_PLAYER_H */
