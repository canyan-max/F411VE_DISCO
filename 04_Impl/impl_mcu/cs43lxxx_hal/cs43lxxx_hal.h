/**
 ******************************************************************************
 *@file               :   cs43lxxx_hal.h
 * 
 *@brief              :   Provide the HAL APIs of description.
 * 
 *@version            :   V1.0 
 * 
 *@note               :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef CS43LXXX_HAL_H
#define CS43LXXX_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -----------------------------------------------------------------*/
#include <stdint.h>                               /* stdint lib header file. */
#include <stddef.h>                               /* stdint lib header file. */
#include "bsp_cs43lxxx_drv.h"

/* define -------------------------------------------------------------------*/

/* typedef ------------------------------------------------------------------*/

/* exported types -----------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/
extern cs43lxxx_hal_ops_t g_cs43lxxx_hal_ops;
extern cs43xxx_drv_t      g_cs43l22_drv;

/* functions ----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* CS43LXXX_HAL_H */
