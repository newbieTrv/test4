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
#ifndef __TY_HAL_OS_H__
#define __TY_HAL_OS_H__

#include <stdint.h>
#include <stdbool.h>
    
#define TY_HAL_OS_portMAX_DELAY      ( 0xffffffffUL )
     
#ifdef __cplusplus
 extern "C" {
#endif 

extern void ty_hal_os_sleep(const uint32_t ms);
extern void ty_hal_os_enter_critical(void);
extern void ty_hal_os_exit_critical(void);

extern bool ty_hal_os_thread_create(void **pp_handle, const char *p_name, void (*p_routine)(void *), \
                    void *p_param, uint16_t stack_size, uint16_t priority);
extern bool ty_hal_os_thread_delete(void *p_handle);
extern bool ty_hal_os_thread_suspend(void *p_handle);
extern bool ty_hal_os_thread_resume(void *p_handle);

extern bool ty_hal_os_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size);
extern bool ty_hal_os_msg_queue_delete(void *p_handle);
extern bool ty_hal_os_msg_queue_peek(void *p_handle, uint32_t *p_msg_num);
extern bool ty_hal_os_msg_queue_send(void *p_handle, void *p_msg, uint32_t wait_ms);
extern bool ty_hal_os_msg_queue_send_from_isr(void *p_handle, void *p_msg);
extern bool ty_hal_os_msg_queue_recv(void *p_handle, void *p_msg, uint32_t wait_ms);

#ifdef __cplusplus
}
#endif 

#endif // __TY_HAL_OS_H__

/* [] END OF FILE */
