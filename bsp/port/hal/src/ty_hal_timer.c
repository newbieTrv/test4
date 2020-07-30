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
#include "ty_hal_timer.h"
#include "board_adapter.h"


int ty_hal_timer_create( \
                void** p_timer_id, \
                uint32_t timeout_value_ms, \
                app_timer_mode_t mode, \
                app_timer_timeout_handler_t timeout_handler) {
    *p_timer_id = xTimerCreate("", pdMS_TO_TICKS(timeout_value_ms), mode, NULL, timeout_handler);
    if (*p_timer_id == NULL)
        return -1;
    
    return 0;
}   

int ty_hal_timer_create_with_timerid( \
                void** p_timer_id, \
                uint32_t timeout_value_ms, app_timer_mode_t mode, \
                void *timer_id, \
                app_timer_timeout_handler_t timeout_handler) {
    *p_timer_id = xTimerCreate("", pdMS_TO_TICKS(timeout_value_ms), mode, timer_id, timeout_handler);
    if (*p_timer_id == NULL)
        return -1;
    
    return 0;
}  

uint32_t ty_hal_timer_get_timerid(void *p_timer_id) {
    return (uint32_t)pvTimerGetTimerID(p_timer_id);
}

int ty_hal_timer_delete(void* timer_id) {
    if (xTimerDelete(timer_id, 0) != pdPASS)
        return -1;
    
    return 0;
}

int ty_hal_timer_start(void* timer_id) {
    if (xTimerStart(timer_id, 0) != pdPASS)
        return -1;
    
    return 0;
}

int ty_hal_timer_restart(void* timer_id, uint32_t timeout_value_ms) {
    int ret = 0;
    
    if (xTimerStop(timer_id, 0) != pdPASS)
        ret = -1;
    else if (xTimerChangePeriod(timer_id, pdMS_TO_TICKS(timeout_value_ms), 0) != pdPASS)
        ret = -1;
    else if (xTimerStart(timer_id, 0) != pdPASS)
        ret = -1;
    
    return ret;
}

int ty_hal_timer_restart_with_context(void* timer_id, uint32_t timeout_value_ms, void *p_context) {
   
}

int ty_hal_timer_stop(void* timer_id) {
	if (xTimerStop(timer_id, 0) != pdPASS)
        return -1;
    
    return 0;
}

/* [] END OF FILE */
