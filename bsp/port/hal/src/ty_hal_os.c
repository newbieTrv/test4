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
#include "ty_hal_os.h"
#include "board_adapter.h"


void ty_hal_os_sleep(const uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void ty_hal_os_enter_critical(void) {   
    taskENTER_CRITICAL(); 
}

void ty_hal_os_exit_critical(void) {
    taskEXIT_CRITICAL();
}

/**************************************************************/
/*************************** thread ***************************/
/**************************************************************/
bool ty_hal_os_thread_create( \
                    void **pp_handle, \
                    const char *p_name, \
                    void (*p_routine)(void *), \
                    void *p_param, \
                    uint16_t stack_size, \
                    uint16_t priority) {
    return xTaskCreate(p_routine, p_name, stack_size, p_param, priority, pp_handle);
}
                    
bool ty_hal_os_thread_delete(void *p_handle) {
    if (p_handle != NULL)
        vTaskDelete(p_handle);
    
    return true;
}

bool ty_hal_os_thread_suspend(void *p_handle) {
    vTaskSuspend(p_handle);
    return true;
}

bool ty_hal_os_thread_resume(void *p_handle) {
    vTaskResume(p_handle);
    return true;
}

/**************************************************************/
/************************** MsgQueue **************************/
/**************************************************************/
bool ty_hal_os_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size) {
    *pp_handle = xQueueCreate(msg_num, msg_size);
    if (*pp_handle == NULL)
        return false;
    
    return true;
}

bool ty_hal_os_msg_queue_delete(void *p_handle) {
    vQueueDelete(p_handle);
    return true;
}

bool ty_hal_os_msg_queue_peek(void *p_handle, uint32_t *p_msg_num) {
    return xQueuePeek(p_handle, p_msg_num, 0);
}

bool ty_hal_os_msg_queue_send(void *p_handle, void *p_msg, uint32_t wait_ms) {
    BaseType_t ret, xHigherPriorityTaskWoken;
    
    if (__get_IPSR() != 0) {
        ret = xQueueSendFromISR(p_handle, p_msg, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
    } else {
        ret = xQueueSend(p_handle, p_msg, (TickType_t)wait_ms);
    }
    
    return ret;
}

bool ty_hal_os_msg_queue_send_from_isr(void *p_handle, void *p_msg) {
    BaseType_t ret, xHigherPriorityTaskWoken;
    
    ret = xQueueSendFromISR(p_handle, p_msg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    
    return ret;
}

bool ty_hal_os_msg_queue_recv(void *p_handle, void *p_msg, uint32_t wait_ms) {
    return xQueueReceive(p_handle, p_msg, (TickType_t)wait_ms);
}

/* [] END OF FILE */
