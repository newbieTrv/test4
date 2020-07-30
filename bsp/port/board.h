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
#ifndef __BOARD_H__
#define __BOARD_H__

#include "cy_gpio.h"
    
/* led mapping table */
typedef struct {
    GPIO_PRT_Type* port;
    uint32_t pin;
} led_mapping_t;

#define LED_NUMS        ( 2 )
#define LED1            ( 0 )
#define LED2            ( 1 )
    
#define BOARD_V1_0

#if defined(BOARD_V1_0)
  #include "board_v1_0.h"
#else
  #error "Board is not defined"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void board_led_init(void);
extern void board_led_turn_on(int led_num);
extern void board_led_turn_off(int led_num);
extern void board_led_toggle(int led_num);
extern void board_led_ctrl_independent(int led_num, uint8_t brightness);

extern void board_init(void);

#ifdef __cplusplus
}
#endif

#endif // __BOARD_H__

/* [] END OF FILE */

