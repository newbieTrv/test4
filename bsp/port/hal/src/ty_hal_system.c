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
#include "ty_hal_system.h"
#include "board_adapter.h"


void ty_hal_system_enter_critical(void) {   
    //uint32_t intrstatues = Cy_SysLib_EnterCriticalSection();
    taskENTER_CRITICAL(); 
}

void ty_hal_system_exit_critical(void) {
    //Cy_SysLib_ExitCriticalSection(intrstatues);
    taskEXIT_CRITICAL();
}

void ty_hal_delay_us(uint32_t us) {
    Cy_SysLib_DelayUs(us);
}

void ty_hal_delay_ms(uint32_t ms) {
    Cy_SysLib_Delay(ms);
}

void ty_hal_system_reboot(void) {
    NVIC_SystemReset();
}

uint32_t ty_hal_system_get_reset_source(void) {
        /*\return The cause of a system reset.
    *
    * | Name                          | Value
    * |-------------------------------|---------------------
    * | CY_SYSLIB_RESET_HWWDT         | 0x00001 (bit0)
    * | CY_SYSLIB_RESET_ACT_FAULT     | 0x00002 (bit1)
    * | CY_SYSLIB_RESET_DPSLP_FAULT   | 0x00004 (bit2)
    * | CY_SYSLIB_RESET_CSV_WCO_LOSS  | 0x00008 (bit3)
    * | CY_SYSLIB_RESET_SOFT          | 0x00010 (bit4)
    * | CY_SYSLIB_RESET_SWWDT0        | 0x00020 (bit5)
    * | CY_SYSLIB_RESET_SWWDT1        | 0x00040 (bit6)
    * | CY_SYSLIB_RESET_SWWDT2        | 0x00080 (bit7)
    * | CY_SYSLIB_RESET_SWWDT3        | 0x00100 (bit8)
    * | CY_SYSLIB_RESET_HFCLK_LOSS    | 0x10000 (bit16)
    * | CY_SYSLIB_RESET_HFCLK_ERR     | 0x20000 (bit17)
    * | CY_SYSLIB_RESET_HIB_WAKEUP    | 0x40000 (bit18)
    */
    uint32_t reset_reason = Cy_SysLib_GetResetReason();
    Cy_SysLib_ClearResetReason();

    return reset_reason;
}

bool ty_hal_system_rst_reason_poweron(void) {
    return (Cy_SysLib_GetResetReason() != CY_SYSLIB_RESET_SOFT) ? (true) : (false);
}

void *ty_hal_system_malloc(const size_t size) {
    return pvPortMalloc(size);
}

void ty_hal_system_free(void* ptr) {
    vPortFree(ptr);
}

int ty_hal_system_get_heapsize(void) {
    return xPortGetFreeHeapSize();
}

uint64_t ty_hal_system_get_uuid(void) {
    return Cy_SysLib_GetUniqueId();
}

void ty_hal_system_log_init(void) {
    /* initialize EasyLogger */
    elog_init();
    
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT,  ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME|ELOG_FMT_FUNC|ELOG_FMT_LINE);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME|ELOG_FMT_FUNC|ELOG_FMT_LINE);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME|ELOG_FMT_FUNC|ELOG_FMT_LINE);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    //elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL);
    //elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL);
    
    /* start EasyLogger */
    elog_start();
}

void ty_hal_system_log_hexdump(const char *name, uint8_t width, uint8_t *buf, uint16_t size) {
    //elog_hexdump(name, width, buf, size);
    elog_hexdump(name, 32, buf, size);
}

/* [] END OF FILE */
