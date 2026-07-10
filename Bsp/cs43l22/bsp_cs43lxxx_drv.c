/**
 ******************************************************************************
 *@file               :   bsp_cs43lxxx_drv.c
 *
 *@brief              :   Provide the HAL APIs of description.
 *
 *@version            :   V1.0
 *
 *@note               :   1 tab == 4 spaces!  2026
 *
 *@pardependencies    :   bsp_cs43lxxx_drv.c
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <stddef.h>           /* stdint lib header file. */
#include "bsp_cs43lxxx_drv.h" /* bsp_cs43lxxx_drv lib header file. */
#include "cs43lxxx_regmap.h"  /* cs43lxxx_regmap lib header file. */
/* define   -----------------------------------------------------------------*/
#define CS43LXXX_READ_REG(dev, reg, buf, len) \
    ((dev)->p_hal_ops->pf_i2c_read_reg((dev)->dev_i2c_adr, (reg), (buf), (len)))

#define CS43LXXX_WRITE_REG(dev, reg, buf, len) \
    ((dev)->p_hal_ops->pf_i2c_write_reg((dev)->dev_i2c_adr, (reg), (buf), (len)))    
/* typedef ------------------------------------------------------------------*/

/* variables ----------------------------------------------------------------*/

/* private  functions  ------------------------------------------------------*/
static cs43lxxx_status_t read_id(cs43xxx_drv_t      *p_drv ,uint8_t *p_id)
{
    if(NULL == p_drv || NULL == p_id)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    uint8_t id = 0x00;
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;
    ret = CS43LXXX_READ_REG(p_drv, CS43L22_REG_ID, &id, 1);
    if(CS43LXXX_STATUS_OK!=ret)
    {
        return ret ;
    }
    id = (id&CS43L22_CHIP_ID_MASK);
    if(CS43L22_CHIP_ID!=id)
    {
        return CS43LXXX_STATUS_ERROR;
    }
    *p_id =  id;
    return  CS43LXXX_STATUS_OK ;
} 
/* exported functions -------------------------------------------------------*/
/**
 * @brief            :  [cs43lxxx_instruct]
 * @retval           :  [ CS43LXXX_STATUS_OK = 0x00U,
                            CS43LXXX_STATUS_ERROR,
                            CS43LXXX_STATUS_BUSY,
                            CS43LXXX_STATUS_TIMEOUT,
                            CS43LXXX_STATUS_ERR_SRC]
 * @param[in]        :  [cs43xxx_drv_t      *p_drv,
                                    cs43lxxx_hal_ops_t *p_hal_ops,
                                    uint8_t             i2c_id]
 */
cs43lxxx_status_t cs43lxxx_instruct(cs43xxx_drv_t      *p_drv,
                                    cs43lxxx_hal_ops_t *p_hal_ops,
                                    uint8_t             i2c_id)
{
    if(NULL == p_drv || NULL == p_hal_ops)
    {
        return CS43LXXX_STATUS_ERR_SRC;
    }
    p_drv->p_hal_ops   = p_hal_ops;
    p_drv->dev_i2c_adr = i2c_id;
    cs43lxxx_status_t ret = CS43LXXX_STATUS_OK;

    return CS43LXXX_STATUS_OK;
}
/* end of  file -------------------------------------------------------------*/
