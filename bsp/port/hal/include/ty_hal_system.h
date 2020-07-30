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
#ifndef __TY_HAL_SYSTEM_H__
#define __TY_HAL_SYSTEM_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
    
#ifdef __cplusplus
 extern "C" {
#endif 

extern void ty_hal_system_enter_critical(void);
extern void ty_hal_system_exit_critical(void);

extern void ty_hal_delay_us(uint32_t us);
extern void ty_hal_delay_ms(uint32_t ms);
extern void ty_hal_system_reboot(void);
extern uint32_t ty_hal_system_get_reset_source(void);
extern bool ty_hal_system_rst_reason_poweron(void);
extern void *ty_hal_system_malloc(const size_t size);
extern void ty_hal_system_free(void* ptr);
extern int ty_hal_system_get_heapsize(void);
extern uint64_t ty_hal_system_get_uuid(void);

extern void ty_hal_system_log_init(void);
extern void ty_hal_system_log_hexdump(const char *name, uint8_t width, uint8_t *buf, uint16_t size);

#ifdef __cplusplus
}
#endif 

#endif // __TY_HAL_SYSTEM_H__

/* [] END OF FILE */
