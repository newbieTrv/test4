
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
#ifndef __TY_HAL_RTC_H__
#define __TY_HAL_RTC_H__

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif 
    
extern void ty_hal_rtc_init(void);
extern uint32_t ty_hal_get_rtc_time(void);
extern uint32_t ty_hal_set_rtc_time(uint32_t timestamp, int32_t timezone);

#ifdef __cplusplus
}
#endif 

#endif // __TY_HAL_RTC_H__

/* [] END OF FILE */