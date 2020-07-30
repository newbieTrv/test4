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
#ifndef __TY_HAL_TIMER_H__
#define __TY_HAL_TIMER_H__
 
#include <stdint.h>

/**@brief Application time-out handler type. */
typedef void (*app_timer_timeout_handler_t)(void * p_context);


/**@brief Timer modes. */
typedef enum {
    TimerOnce = 0,  /**< The timer will expire only once. */
    TimerPeriodic,  /**< The timer will restart each time it expires. */
} app_timer_mode_t;


/**@brief Timer struct. */
#pragma pack(1) 
typedef struct {
	uint32_t period;	
	app_timer_mode_t type;
    int id;
    const char name[32];
	void (*cb)(void *);
} xTimeParas;
#pragma pack() 


#ifdef __cplusplus
 extern "C" {
#endif 
   
extern 
int ty_hal_timer_create( \
                void** p_timer_id, \
                uint32_t timeout_value_ms, \
                app_timer_mode_t mode, \
                app_timer_timeout_handler_t timeout_handler);
extern int ty_hal_timer_create_with_timerid( \
                void** p_timer_id, \
                uint32_t timeout_value_ms, app_timer_mode_t mode, \
                void *timer_id, \
                app_timer_timeout_handler_t timeout_handler);
extern uint32_t ty_hal_timer_get_timerid(void *p_timer_id);

extern int ty_hal_timer_delete(void* timer_id);
extern int ty_hal_timer_start(void* timer_id);
extern int ty_hal_timer_restart(void* timer_id, uint32_t timeout_value_ms);
extern int ty_hal_timer_restart_with_context(void* timer_id, uint32_t timeout_value_ms, void *p_context);
extern int ty_hal_timer_stop(void* timer_id);

#ifdef __cplusplus
}
#endif 

#endif // __TY_HAL_TIMER_H__

/* [] END OF FILE */
