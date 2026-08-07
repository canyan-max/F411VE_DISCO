/**
 ******************************************************************************
 *@file               :   cs43lxxx_regmap.h
 *@brief              :   Provide the HAL APIs of description.
 *@version            :   V1.0 
 *@note               :   1 tab == 4 spaces!  2026
 ******************************************************************************
 */
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef CS43LXXX_REGMAP_H
#define CS43LXXX_REGMAP_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* Includes -----------------------------------------------------------------*/

/* define -------------------------------------------------------------------*/
#define   CS43L22_REG_ID                  (0x01U)
#define   CS43L22_REG_POWER_CTL1          (0x02U)
#define   CS43L22_REG_POWER_CTL2          (0x04U)
#define   CS43L22_REG_CLOCKING_CTL        (0x05U)
#define   CS43L22_REG_INTERFACE_CTL1      (0x06U)
#define   CS43L22_REG_INTERFACE_CTL2      (0x07U)
#define   CS43L22_REG_PASSTHR_A_SELECT    (0x08U)
#define   CS43L22_REG_PASSTHR_B_SELECT    (0x09U)
#define   CS43L22_REG_ANALOG_ZC_SR_SETT   (0x0AU)
#define   CS43L22_REG_PASSTHR_GANG_CTL    (0x0CU)
#define   CS43L22_REG_PLAYBACK_CTL1       (0x0DU)
#define   CS43L22_REG_MISC_CTL            (0x0EU)
#define   CS43L22_REG_PLAYBACK_CTL2       (0x0FU)
#define   CS43L22_REG_PASSTHR_A_VOL       (0x14U)
#define   CS43L22_REG_PASSTHR_B_VOL       (0x15U)
#define   CS43L22_REG_PCMA_VOL            (0x1AU)
#define   CS43L22_REG_PCMB_VOL            (0x1BU)
#define   CS43L22_REG_BEEP_FREQ_ON_TIME   (0x1CU)
#define   CS43L22_REG_BEEP_VOL_OFF_TIME   (0x1DU)
#define   CS43L22_REG_BEEP_TONE_CFG       (0x1EU)
#define   CS43L22_REG_TONE_CTL            (0x1FU)
#define   CS43L22_REG_MASTER_A_VOL        (0x20U)
#define   CS43L22_REG_MASTER_B_VOL        (0x21U)
#define   CS43L22_REG_HEADPHONE_A_VOL     (0x22U)
#define   CS43L22_REG_HEADPHONE_B_VOL     (0x23U)
#define   CS43L22_REG_SPEAKER_A_VOL       (0x24U)
#define   CS43L22_REG_SPEAKER_B_VOL       (0x25U)
#define   CS43L22_REG_CH_MIXER_SWAP       (0x26U)
#define   CS43L22_REG_LIMIT_CTL1          (0x27U)
#define   CS43L22_REG_LIMIT_CTL2          (0x28U)
#define   CS43L22_REG_LIMIT_ATTACK_RATE   (0x29U)
#define   CS43L22_REG_OVF_CLK_STATUS      (0x2EU)
#define   CS43L22_REG_BATT_COMPENSATION   (0x2FU)
#define   CS43L22_REG_VP_BATTERY_LEVEL    (0x30U)
#define   CS43L22_REG_SPEAKER_STATUS      (0x31U)
#define   CS43L22_REG_TEMPMONITOR_CTL     (0x32U)
#define   CS43L22_REG_THERMAL_FOLDBACK    (0x33U)
#define   CS43L22_REG_CHARGE_PUMP_FREQ    (0x34U)
/* typedef ------------------------------------------------------------------*/

/* exported types -----------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/

/* functions ----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* CS43LXXX_REGMAP_H */
