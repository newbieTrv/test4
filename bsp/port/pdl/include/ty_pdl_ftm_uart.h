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
#ifndef __TY_PDL_FTM_UART_H__
#define __TY_PDL_FTM_UART_H__

#include "board_adapter.h"

#define FTM_UART_BAUD_RATE       ( 9600 )
    
/*
 *  ty_pdl_ftm_uart_ops_t 对外接口
*/
typedef struct {
    bool (*ftm_uart_open)(uint32_t uart_baud);
    bool (*ftm_uart_close)(void);
    void (*ftm_uart_send)(uint8_t *data, uint16_t size);
} ty_pdl_ftm_uart_ops_t;

extern ty_pdl_ftm_uart_ops_t ty_pdl_ftm_uart_ops;
extern bool Task_Factory_Test_Tickless_Idle_Readiness(void);

#ifdef __cplusplus
 extern "C" {
#endif 


#ifdef __cplusplus
}
#endif 

#endif // __TY_PDL_FTM_UART_H__

/* [] END OF FILE */
