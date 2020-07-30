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
#ifndef __BOARD_ADAPTER_H__
#define __BOARD_ADAPTER_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
    
// board
#include "board.h"

// platform 
/* Psoc creator General */
#include "project.h"   
#include "cy_sysint.h"
#include "cy_rtc.h" 
#include "cy_flash.h"
#include "cy_mcwdt.h"    
#include "cy_syslib.h"    
#include "cy_gpio.h"
#include "uart_debug.h"

// rtos
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"   
     
// hal 
#include "ty_hal_rtc.h"
#include "ty_hal_flash.h"
#include "ty_hal_wdt.h"
#include "ty_hal_os.h"  
#include "ty_hal_timer.h"    
#include "ty_hal_system.h"  

// pdl 
#include "ty_pdl_ble.h"
#include "ty_pdl_ftm_uart.h"
    
// tools 
#include "elog.h"

    
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // __BOARD_ADAPTER_H__

/* [] END OF FILE */
