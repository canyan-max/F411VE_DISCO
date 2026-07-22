/**
 ******************************************************************************
 *@file    :   sd_manager.h
 *@brief   :   SD card resource manager.
 *             Owns the FATFS instance and file handles. Callers never touch
 *             FatFS types directly — use the API below instead.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include <stdint.h>
#include "media_src.h"
/* typedef ------------------------------------------------------------------*/
typedef enum
{
    SD_OK = 0,
    SD_ERR_NOT_MOUNTED,
    SD_ERR_MOUNT,
    SD_ERR_OPEN,
    SD_ERR_READ,
    SD_ERR_WRITE,
} sd_status_t;

/* functions ----------------------------------------------------------------*/
/**
 * @brief            :  [sd_manager_mount]
 */
sd_status_t sd_manager_mount(void);
/**
 * @brief            :  [sd_manager_open]
 * @param[in]        :  [const char *p_path]
 */
sd_status_t sd_manager_open(const char *p_path);
/**
 * @brief            :  [sd_manager_close]
 */
void sd_manager_close(void);
/**
 * @brief            :  [sd_manager_get_src]
 */
sd_status_t sd_manager_get_src(media_src_t *p_src);

/* Future expansion (write support):
 * sd_status_t sd_manager_open_write(const char *p_path);
 * sd_status_t sd_manager_write(const void *p_data, uint32_t len);
 */

#ifdef __cplusplus
}
#endif

#endif /* SD_MANAGER_H */
