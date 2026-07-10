/**
 ******************************************************************************
 *@file               :   bsp_cs43lxxx_drv.h
 *
 *@brief              :   Provide the HAL APIs of description.
 *
 *@version            :   V1.0
 *
 *@note               :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef BSP_CS43LXXX_DRV_H
#define BSP_CS43LXXX_DRV_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes -----------------------------------------------------------------*/
#include <stdint.h> /* stdint lib header file. */
#include <stddef.h> /* stdint lib header file. */

/* define -------------------------------------------------------------------*/
#define CS43XXX_NOT_INIT              (0x00U) /* CS43XXX_NOT_INIT. */
#define CS43XXX_IS_INIT               (0x01U) /* CS43XXX_IS_INIT. */
#define CS43XXX_I2C_ADDR_7BIT         (0x94U) /* CS43LXXX I2C address.*/
#define CS43L22_CHIP_ID_MASK          (0xF8U)
#define CS43L22_CHIP_ID               (0xE0U)
/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER         (1U)
#define OUTPUT_DEVICE_HEADPHONE       (2U)
#define OUTPUT_DEVICE_BOTH            (3U)
#define OUTPUT_DEVICE_AUTO            (4U)

/* Volume Levels values */
#define DEFAULT_VOLMIN                (0x00U)
#define DEFAULT_VOLMAX                (0xFFU)
#define DEFAULT_VOLSTEP               (0x04U)

#define AUDIO_PAUSE                   (0U)
#define AUDIO_RESUME                  (1U)

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 (1U)
#define CODEC_PDWN_SW                 (2U)

/* MUTE commands */
#define AUDIO_MUTE_ON                 (1U)
#define AUDIO_MUTE_OFF                (0U)

/* typedef ------------------------------------------------------------------*/
typedef enum CS43LXXX_STATUS_T
{
    // CS43LXXX I2C operations is OK. 
    CS43LXXX_STATUS_OK = 0x00U,
   //  CS43LXXX I2C operations is ERROR. 
    CS43LXXX_STATUS_ERROR,
    //CS43LXXX I2C operations is BUSY.
    CS43LXXX_STATUS_BUSY,
   //  CS43LXXX I2C operations is TIMEOUT. 
    CS43LXXX_STATUS_TIMEOUT,
    // CS43LXXX_STATUS_ERR_SRC
    CS43LXXX_STATUS_ERR_SRC
} cs43lxxx_status_t;

typedef struct CS43LXXX_HAL_OPS_T
{
    // ops table of CS43LXXX I2C operations tables

    //  I2C write operation function pointer.
    cs43lxxx_status_t (*pf_i2c_write_reg)(uint8_t  dev_addr,
                                         uint16_t reg_addr,
                                         uint8_t *p_data,
                                         uint16_t len);
    //I2C read operation function pointer. 
    cs43lxxx_status_t (*pf_i2c_read_reg)(uint8_t  dev_addr,
                                        uint16_t reg_addr,
                                        uint8_t *p_data,
                                        uint16_t len);
    // I2S tramsmit operation function pointer.                                 
    cs43lxxx_status_t (*pf_i2s_transmit_with_dma)(uint16_t *p_buffer,
                                                 uint16_t  size);
    // power control operation function pointer.
    void (*pf_power_control)(uint8_t state);
    // system delay operation function pointer.
    void (*pf_delay_ms)(uint32_t ms);                                                 
} cs43lxxx_hal_ops_t;

typedef struct CS43XXX_DRV_T
{
    /* ops table of CS43LXXX I2C operations tables*/
    /* hal operations table pointer. */
    cs43lxxx_hal_ops_t *p_hal_ops;
    /* drv data*/
    uint8_t is_init;
    uint8_t dev_i2c_adr;
} cs43xxx_drv_t;

/* exported types -----------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/

/* functions ----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* BSP_CS43LXXX_DRV_H */
