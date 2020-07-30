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
#include "ty_pdl_ftm_uart.h"
    
bool ty_pdl_ftm_uart_open(uint32_t uart_baud);
bool ty_pdl_ftm_uart_close(void);
void ty_pdl_ftm_uart_send(uint8_t *data, uint16_t size);
void ty_pdl_ftm_irq_config(void);

static void ty_pdl_ftm_uart_interrupt(void);
static void ty_pdl_ftm_irq_interrupt(void);

ty_pdl_ftm_uart_ops_t ty_pdl_ftm_uart_ops = {
    .ftm_uart_open       = &ty_pdl_ftm_uart_open,
    .ftm_uart_close      = &ty_pdl_ftm_uart_close,
    .ftm_uart_send       = &ty_pdl_ftm_uart_send,
};

#define UART_RCV_MAXSIZE        ( 256 )
static uint8_t uart_rcv_buf[UART_RCV_MAXSIZE] = {0};
static uint16_t uart_rcv_buf_len = 0;

/* Timer handler */
static void *xUartRcvTimeoutTimerHandle = NULL;
static void xUartRcvTimeoutTimerCallback(void *xTimer); 

static void uart_timeout_timer_init(void) {
    ty_hal_timer_create(&xUartRcvTimeoutTimerHandle, 5, TimerOnce, xUartRcvTimeoutTimerCallback);
    if (xUartRcvTimeoutTimerHandle == NULL) 
        log_d("create uart recv timeout timer failed\n");
} 
    
static void uart_timeout_timer_reset(void) {
    BaseType_t xHigherPriorityTaskWoken; 
    
    xTimerResetFromISR(xUartRcvTimeoutTimerHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void xUartRcvTimeoutTimerCallback(void *xTimer) {
    (void)(xTimer);
    
    extern uint8_t tuya_ble_common_uart_receive_data(uint8_t *p_data, uint16_t len);
    tuya_ble_common_uart_receive_data(uart_rcv_buf, uart_rcv_buf_len);
    
    uart_rcv_buf_len = 0;
    memset(uart_rcv_buf, 0x00, UART_RCV_MAXSIZE);
}

bool ty_pdl_ftm_uart_open(uint32_t uart_baud) {
    if (xUartRcvTimeoutTimerHandle == NULL) 
        uart_timeout_timer_init();
        
    /* Connect SCBx UART function to pins */
    Cy_GPIO_SetHSIOM(FTM_UART_rx_PORT, FTM_UART_rx_NUM, HSIOM_SEL_ACT_6);
    Cy_GPIO_SetHSIOM(FTM_UART_tx_PORT, FTM_UART_tx_NUM, HSIOM_SEL_ACT_6);

    /* Configure pins for UART operation */
    Cy_GPIO_SetDrivemode(FTM_UART_rx_PORT, FTM_UART_rx_NUM, CY_GPIO_DM_HIGHZ);
    Cy_GPIO_SetDrivemode(FTM_UART_tx_PORT, FTM_UART_tx_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    
    FTM_UART_Start();
    
    /* Unmasking only the RX fifo not empty interrupt bit */
    FTM_UART_HW->INTR_RX_MASK = SCB_INTR_RX_MASK_NOT_EMPTY_Msk;
    (void)(uart_baud);
    
    /* Interrupt Settings for UART */    
    Cy_SysInt_Init(&FTM_UART_SCB_IRQ_cfg, ty_pdl_ftm_uart_interrupt);
    
    /* Enable the interrupt */
    NVIC_EnableIRQ(FTM_UART_SCB_IRQ_cfg.intrSrc);
    log_d("ftm uart open\n");
    return true;
}

bool ty_pdl_ftm_uart_close(void) {
    if (1U == FTM_UART_initVar) {
        FTM_UART_Disable();
        FTM_UART_DeInit();
        
        /* Config uart pins as GPIO pins */
        Cy_GPIO_Pin_FastInit(FTM_UART_rx_PORT, FTM_UART_rx_NUM, CY_GPIO_DM_STRONG_IN_OFF, 1, HSIOM_SEL_GPIO);
        Cy_GPIO_Set(FTM_UART_rx_PORT, FTM_UART_rx_NUM);
        
        Cy_GPIO_Pin_FastInit(FTM_UART_tx_PORT, FTM_UART_tx_NUM, CY_GPIO_DM_STRONG_IN_OFF, 1, HSIOM_SEL_GPIO);
        Cy_GPIO_Set(FTM_UART_tx_PORT, FTM_UART_tx_NUM);
        
        FTM_UART_initVar = 0U;
    }

    log_d("ftm uart close\n");
    return true;
}

void ty_pdl_ftm_uart_send(uint8_t *data, uint16_t size) {
    FTM_UART_PutArrayBlocking(data, size);
    elog_hexdump("\r\nftm snd", 32, data, size);
}

static void ty_pdl_ftm_uart_interrupt(void) {
    uint8_t rcv_data;

    /* Check for "RX fifo not empty interrupt" */
    if ((FTM_UART_HW->INTR_RX_MASKED & SCB_INTR_RX_MASKED_NOT_EMPTY_Msk ) != 0) {
        /* Clear UART "RX fifo not empty interrupt" */
		FTM_UART_HW->INTR_RX = FTM_UART_HW->INTR_RX & SCB_INTR_RX_NOT_EMPTY_Msk;        
            
        /* Get the character from terminal */
		rcv_data = Cy_SCB_UART_Get(FTM_UART_HW);

        //log_d("rcv->[%02x]\r\n", rcv_data);
        uart_rcv_buf[uart_rcv_buf_len++] = rcv_data;
        if (uart_rcv_buf_len >= UART_RCV_MAXSIZE)
            uart_rcv_buf_len = 0;

        BaseType_t xHigherPriorityTaskWoken; 
        xTimerResetFromISR(xUartRcvTimeoutTimerHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else {
        /* Error if any other interrupt occurs */
    }
}

bool Task_Factory_Test_Tickless_Idle_Readiness (void) {
    /* Return "true" if the fp uart is on, "false" otherwise */
    return (FTM_UART_initVar == 0) ? true : false;
}

/* [] END OF FILE */
