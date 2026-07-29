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
#include "platform_error.h"

/* define   -----------------------------------------------------------------*/
#define SD_DRIVER_LETTER "0:"
/* functions ----------------------------------------------------------------*/
/**
 * @brief            :  [sd_manager_mount]
 */
platform_err_t sd_manager_mount(const char *path);
/**
 * @brief            :  [sd_manager_open]
 * @param[in]        :  [const char *p_path]
 */
platform_err_t sd_manager_open(const char *p_path);
/**
 * @brief            :  [sd_manager_close]
 */
void sd_manager_close(void);
/**
 * @brief            :  [sd_manager_get_src]
 */
platform_err_t sd_manager_get_src(media_src_t *p_src);
/**
 * @brief            :  [sd_manager_list_dir]
 * @param[in]        :  [const char *p_path]   directory path, e.g. "0:/"
 * @param[out]       :  [FILINFO *p_list]       caller-provided array
 * @param[in]        :  [uint8_t max_count]     capacity of p_list
 * @param[out]       :  [uint8_t *p_found]      actual number of files found
 */
platform_err_t sd_manager_list_dir(const char *p_path,
                                 FILINFO    *p_list,
                                 uint8_t     max_count,
                                 uint8_t    *p_found);
/**
 * @brief            :  [sd_manager_open_write]
 * @param[in]        :  [const char *p_path]  file path, e.g. "0:/recv.bin"
 * @note             :  FA_OPEN_APPEND: create if not exist, append if exist
 */
platform_err_t sd_manager_open_write(const char *p_path);
/**
 * @brief            :  [sd_manager_write]
 * @param[in]        :  [const uint8_t *p_buf]   data to write
 * @param[in]        :  [uint32_t len]            byte count (keep <= 512 to
 *                                                avoid blocking the read path)
 * @param[out]       :  [uint32_t *p_written]     actual bytes written, or NULL
 */
platform_err_t sd_manager_write(const uint8_t *p_buf,
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
