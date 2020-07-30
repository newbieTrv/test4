/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef __TY_HAL_WDT_H__
#define __TY_HAL_WDT_H__

#include <stdint.h>
    
#ifdef __cplusplus
    extern "C" {
#endif 

extern int ty_hal_watchdog_start(uint32_t period);
extern void ty_hal_watchdog_refresh(void);
extern void ty_hal_watchdog_stop(void);

#ifdef __cplusplus
}
#endif 

#endif // __TY_HAL_WDT_H__

/* [] END OF FILE */
