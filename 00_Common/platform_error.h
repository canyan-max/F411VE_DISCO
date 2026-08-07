/**
 ******************************************************************************
 *@file    :   platform_error.h
 *@brief   :   Unified error codes for all layers.
 *             All public APIs return platform_err_t. Upper layers include this
 *             file via the Platform include path — never include HAL headers.
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef PLATFORM_ERROR_H
#define PLATFORM_ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

/* typedef ------------------------------------------------------------------*/
typedef enum
{
    PLATFORM_ERR_OK           = 0,
    PLATFORM_ERR_PARAM        = 1,
    PLATFORM_ERR_BUSY         = 2,
    PLATFORM_ERR_TIMEOUT      = 3,
    PLATFORM_ERR_FAIL         = 4,
    PLATFORM_ERR_NOT_INIT     = 5,
    PLATFORM_ERR_NO_RESOURCE  = 6,
    PLATFORM_ERR_RESERVED     = 0x7FFFFFFF,
} platform_err_t;

/* macros -------------------------------------------------------------------*/
#define PLATFORM_IS_OK(err)     ((err) == PLATFORM_ERR_OK)
#define PLATFORM_IS_ERR(err)    ((err) != PLATFORM_ERR_OK)

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_ERROR_H */
