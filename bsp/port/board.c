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
#include "board.h"
#include "board_adapter.h"

led_mapping_t led_mapping[LED_NUMS] = {
    {LED1_PIN_PORT, LED1_PIN_NUM},
    {LED2_PIN_PORT, LED2_PIN_NUM},
};

void board_led_init(void) {
    for (int i=0; i<LED_NUMS; i++) {
        Cy_GPIO_Pin_FastInit(led_mapping[i].port, led_mapping[i].pin, CY_GPIO_DM_STRONG, 1, HSIOM_SEL_GPIO); 
        Cy_GPIO_Set(led_mapping[i].port, led_mapping[i].pin);
    }
}   

void board_led_turn_on(int led_num) {
    Cy_GPIO_Clr(led_mapping[led_num].port, led_mapping[led_num].pin);
}   

void board_led_turn_off(int led_num) {
    Cy_GPIO_Set(led_mapping[led_num].port, led_mapping[led_num].pin);
}   

void board_led_toggle(int led_num) {
    Cy_GPIO_Inv(led_mapping[led_num].port, led_mapping[led_num].pin);
}  

void board_led_ctrl_independent(int led_num, uint8_t brightness) {
    if (brightness != 0)
        Cy_GPIO_Clr(led_mapping[led_num].port, led_mapping[led_num].pin);
    else 
        Cy_GPIO_Set(led_mapping[led_num].port, led_mapping[led_num].pin);
} 

static void board_config_unused_pins(void) {
    /* Set Unused pins to Alg HI-Z for low power. */
    
    // TODO .. for example
    //Cy_GPIO_SetDrivemode(GPIO_PRT0, 2, CY_GPIO_DM_ANALOG);
    //Cy_GPIO_SetDrivemode(GPIO_PRT0, 3, CY_GPIO_DM_ANALOG);
}

void board_init(void) {
    board_config_unused_pins();
    board_led_init();
    
    ty_hal_system_log_init();
    ty_hal_rtc_init();
    ty_hal_flash_init();
    //ty_hal_watchdog_start(60);
}

/* [] END OF FILE */
