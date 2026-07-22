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
static uint8_t s_mounted = 0;

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
sd_status_t sd_manager_mount(void)
{
    FRESULT fr = f_mount(&s_fs, USERPath, 1);
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

/* end of file --------------------------------------------------------------*/
