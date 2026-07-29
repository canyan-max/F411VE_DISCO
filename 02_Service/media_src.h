/**
 ******************************************************************************
 *@file    :   media_src.h
 *@brief   :   Generic offset-based read interface for media data sources.
 *             Any storage backend (SD card, SPI flash, USB, RAM array) can
 *             implement pf_read and hand the struct to a decoder.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef MEDIA_SRC_H
#define MEDIA_SRC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct
{
    uint32_t (*pf_read)(void    *p_ctx,
                        uint32_t offset,
                        uint8_t *p_buf,
                        uint32_t len);
    void    *p_ctx;
    uint32_t total_size;
} media_src_t;

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_SRC_H */
