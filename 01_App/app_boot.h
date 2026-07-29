/**
 ******************************************************************************
 *@file    :   app_boot.h
 *@brief   :   Application boot orchestrator.
 *             Single entry point for all task init and create calls.
 *             freertos.c USER CODE only needs to call app_boot_init().
 *@version :   V1.0
 *@note    :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
#ifndef APP_BOOT_H
#define APP_BOOT_H

#ifdef __cplusplus
extern "C"
{
#endif

void app_boot_init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BOOT_H */
