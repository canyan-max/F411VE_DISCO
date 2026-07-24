/**
 ******************************************************************************
 *@file    :   sd_manager.c
 *@brief   :   SD card resource manager.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include "sd_manager.h"
#include "fatfs.h"

/* private variables --------------------------------------------------------*/
static FATFS   s_fs;
static FIL     s_file;
static FIL     s_wfile;
static uint8_t s_mounted    = 0;
static uint8_t s_wfile_open = 0;

/* private functions --------------------------------------------------------*/
static uint32_t sd_src_read(void    *p_ctx,
                            uint32_t offset,
                            uint8_t *p_buf,
                            uint32_t len)
{
    FIL    *p_file = (FIL *)p_ctx;
    UINT    br;
    FRESULT fr;

    if (f_tell(p_file) != (FSIZE_t)offset)
    {
        fr = f_lseek(p_file, (FSIZE_t)offset);
        if (fr != FR_OK)
        {
            return 0U;
        }
    }
    fr = f_read(p_file, p_buf, (UINT)len, &br);
    if (fr != FR_OK)
    {
        return 0U;
    }
    return (uint32_t)br;
}

/* exported functions -------------------------------------------------------*/
/**
 * @brief            :  [sd_manager_mount]
 */
sd_status_t sd_manager_mount(const char * path)
{
    FRESULT fr = f_mount(&s_fs, path, 1);
    if (fr != FR_OK)
    {
        return SD_ERR_MOUNT;
    }
    s_mounted = 1;
    return SD_OK;
}
/**
 * @brief            :  [sd_manager_open]
 * @param[in]        :  [const char *p_path]
 */
sd_status_t sd_manager_open(const char *p_path)
{
    if (!s_mounted)
    {
        return SD_ERR_NOT_MOUNTED;
    }
    FRESULT fr = f_open(&s_file, p_path, FA_READ);
    if (fr != FR_OK)
    {
        return SD_ERR_OPEN;
    }
    return SD_OK;
}
/**
 * @brief            :  [sd_manager_close]
 */
void sd_manager_close(void)
{
    f_close(&s_file);
}
/**
 * @brief            :  [sd_manager_get_src]
 */
sd_status_t sd_manager_get_src(media_src_t *p_src)
{
    p_src->pf_read    = sd_src_read;
    p_src->p_ctx      = &s_file;
    p_src->total_size = (uint32_t)f_size(&s_file);
    return SD_OK;
}

/**
 * @brief            :  [sd_manager_list_dir]
 */
sd_status_t sd_manager_list_dir(const char *p_path,
                                 FILINFO    *p_list,
                                 uint8_t     max_count,
                                 uint8_t    *p_found)
{
    if (!s_mounted)
    {
        return SD_ERR_NOT_MOUNTED;
    }

    DIR     dir;
    FRESULT fr;
    uint8_t count = 0;

    fr = f_opendir(&dir, p_path);
    if (fr != FR_OK)
    {
        return SD_ERR_OPEN;
    }

    while (count < max_count)
    {
        fr = f_readdir(&dir, &p_list[count]);
        if (fr != FR_OK || p_list[count].fname[0] == '\0')
        {
            break;
        }
        if (p_list[count].fattrib & AM_DIR)
        {
            continue;
        }
        count++;
    }

    f_closedir(&dir);
    if (p_found != NULL)
    {
        *p_found = count;
    }
    return SD_OK;
}

/**
 * @brief            :  [sd_manager_open_write]
 */
sd_status_t sd_manager_open_write(const char *p_path)
{
    if (!s_mounted)
    {
        return SD_ERR_NOT_MOUNTED;
    }
    FRESULT fr = f_open(&s_wfile, p_path, FA_WRITE | FA_OPEN_APPEND);
    if (fr != FR_OK)
    {
        return SD_ERR_OPEN;
    }
    s_wfile_open = 1;
    return SD_OK;
}

/**
 * @brief            :  [sd_manager_write]
 */
sd_status_t sd_manager_write(const uint8_t *p_buf,
                              uint32_t       len,
                              uint32_t      *p_written)
{
    if (!s_wfile_open)
    {
        return SD_ERR_NOT_OPEN;
    }
    UINT    bw;
    FRESULT fr = f_write(&s_wfile, p_buf, (UINT)len, &bw);
    if (fr != FR_OK)
    {
        return SD_ERR_WRITE;
    }
    if (p_written != NULL)
    {
        *p_written = (uint32_t)bw;
    }
    return SD_OK;
}

/**
 * @brief            :  [sd_manager_close_write]
 */
void sd_manager_close_write(void)
{
    if (s_wfile_open)
    {
        f_close(&s_wfile);
        s_wfile_open = 0;
    }
}

/* end of file --------------------------------------------------------------*/
