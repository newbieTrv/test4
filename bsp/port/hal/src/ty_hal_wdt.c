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
#include "ty_hal_wdt.h"
#include "board_adapter.h"

static cy_stc_mcwdt_config_t config = {
    .c0Match = 32768,
    .c1Match = 10,
    .c0Mode = CY_MCWDT_MODE_NONE,
    .c1Mode = CY_MCWDT_MODE_RESET,
    .c2ToggleBit = 16,
    .c2Mode = CY_MCWDT_MODE_NONE,
    .c0ClearOnMatch = true,
    .c1ClearOnMatch = true,
    .c0c1Cascade = true,
    .c1c2Cascade = false
};

int ty_hal_watchdog_start(uint32_t period /*unit: s*/) {
    config.c1Match = period;

	Cy_MCWDT_Init(MCWDT_STRUCT1, &MCWDT1_config);
	Cy_MCWDT_Enable(MCWDT_STRUCT1, (CY_MCWDT_CTR0 | CY_MCWDT_CTR1), MCWDT_TWO_LF_CLK_CYCLES_DELAY);
    return 0;
}

void ty_hal_watchdog_refresh(void) {
    Cy_MCWDT_ResetCounters(MCWDT_STRUCT1, (CY_MCWDT_CTR0 | CY_MCWDT_CTR1), MCWDT_TWO_LF_CLK_CYCLES_DELAY);
}

void ty_hal_watchdog_stop(void) {
    Cy_MCWDT_Disable(MCWDT_STRUCT1, (CY_MCWDT_CTR0 | CY_MCWDT_CTR1), MCWDT_TWO_LF_CLK_CYCLES_DELAY);
    Cy_MCWDT_DeInit(MCWDT_HW);
}

/* [] END OF FILE */ 
