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
#include "ff.h"
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
    SD_ERR_NOT_OPEN,
} sd_status_t;

/* define   -----------------------------------------------------------------*/
#define SD_DRIVER_LETTER "0:"
/* functions ----------------------------------------------------------------*/
/**
 * @brief            :  [sd_manager_mount]
 */
sd_status_t sd_manager_mount(const char *path);
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
/**
 * @brief            :  [sd_manager_list_dir]
 * @param[in]        :  [const char *p_path]   directory path, e.g. "0:/"
 * @param[out]       :  [FILINFO *p_list]       caller-provided array
 * @param[in]        :  [uint8_t max_count]     capacity of p_list
 * @param[out]       :  [uint8_t *p_found]      actual number of files found
 */
sd_status_t sd_manager_list_dir(const char *p_path,
                                 FILINFO    *p_list,
                                 uint8_t     max_count,
                                 uint8_t    *p_found);
/**
 * @brief            :  [sd_manager_open_write]
 * @param[in]        :  [const char *p_path]  file path, e.g. "0:/recv.bin"
 * @note             :  FA_OPEN_APPEND: create if not exist, append if exist
 */
sd_status_t sd_manager_open_write(const char *p_path);
/**
 * @brief            :  [sd_manager_write]
 * @param[in]        :  [const uint8_t *p_buf]   data to write
 * @param[in]        :  [uint32_t len]            byte count (keep <= 512 to
 *                                                avoid blocking the read path)
 * @param[out]       :  [uint32_t *p_written]     actual bytes written, or NULL
 */
sd_status_t sd_manager_write(const uint8_t *p_buf,
                              uint32_t       len,
                              uint32_t      *p_written);
/**
 * @brief            :  [sd_manager_close_write]
 */
void sd_manager_close_write(void);

#ifdef __cplusplus
}
#endif

#endif /* SD_MANAGER_H */
