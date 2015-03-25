/** 
 * @file 
 *   hal_hw_api.h 
 *
 * @par Project:
 *   MFlexVideo 
 *
 * @par Description:
 *   Hardware Abstraction Layer APIs
 *
 * @par Author:
 *   Jackal Chen (mtk02532)
 *
 * @par $Revision: #3 $
 * @par $Modtime:$
 * @par $Log:$
 *
 */

#ifndef _HAL_HW_API_H_
#define _HAL_HW_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "val_types.h"
#include "hal_types.h"


///////////////////////////////////////////////
////// HW dependent function
///////////////////////////////////////////////
/**
* @par Function       
*   eHalHwEnablePower
* @par Description    
*   enable hw clock  
* @param              
*   a_prParam         [IN/OUT]   The HAL_CLOCK_T structure
* @param              
*   a_u4ParamSize     [IN]       The size of HAL_CLOCK_T
* @par Returns        
*   VAL_RESULT_T
*/
VAL_RESULT_T eHalHwPowerCtrl(
    HAL_POWER_T     *a_prParam, 
    VAL_UINT32_T    a_u4ParamSize
    );

/**
* @par Function       
*   eHalHwEnableClock
* @par Description    
*   enable hw clock  
* @param              
*   a_prParam         [IN/OUT]   The HAL_CLOCK_T structure
* @param              
*   a_u4ParamSize     [IN]       The size of HAL_CLOCK_T
* @par Returns        
*   VAL_RESULT_T
*/
VAL_RESULT_T eHalHwEnableClock(
    HAL_CLOCK_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

/**
* @par Function       
*   eHalHwDisableClock
* @par Description    
*   disable hw clock  
* @param              
*   a_prParam         [IN/OUT]   The HAL_CLOCK_T structure
* @param              
*   a_u4ParamSize     [IN]       The size of HAL_CLOCK_T
* @par Returns        
*   VAL_RESULT_T
*/
VAL_RESULT_T eHalHwDisableClock(
    HAL_CLOCK_T *a_prParam, 
    VAL_UINT32_T a_u4ParamSize
);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _HAL_HW_API_H_
