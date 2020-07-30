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
#include "ty_hal_flash.h"
#include "board_adapter.h"

void ty_hal_flash_init(void) {
    // This function is called in the SystemInit() function,
    //Cy_Flash_Init();
}

int ty_hal_flash_erase_sector(const uint32_t addr) {
    int ret = 0;
    
    if (Cy_Flash_EraseRow(addr) != CY_FLASH_DRV_SUCCESS) {
        log_d("ty hal flash erase failed\r\n");
        ret = -1;
    }
   
    return ret;
}

// Address must match row start address. Cy_Flash_BoundsCheck()
// data必须为rowsize，在cy_fstorage_write_row函数内转化下
// Data to be programmed must be located in the SRAM memory region.
// The size of the data array must be equal to the flash row size. 
int ty_hal_flash_write_sector(const uint32_t addr, const uint8_t *src, uint32_t size) {
    int ret = 0;
    
    if ((addr > CALCULATE_FLASH_ADDRESS(LAST_FLASH_ROW)) || \
        (src == NULL) || 
        (size > CY_FLASH_SIZEOF_ROW)) 
        return -1;
   
    /* This is done in three steps - pre-program, erase and then program flash row with the input data */
    if (Cy_Flash_ProgramRow((uint32_t)addr, (const uint32_t *)src) != CY_FLASH_DRV_SUCCESS) { 
        log_d("ty hal flash write failed\r\n");
        ret = -1;
    }
    
    return ret;
}

int ty_hal_flash_read_sector(const uint32_t addr, uint8_t *dst, uint32_t size) {
    if ((addr > CALCULATE_FLASH_ADDRESS(LAST_FLASH_ROW)) || (dst == NULL)) {
        log_d("ty hal flash read failed\r\n");
        return -1;
    }
    
    // Before reading data from previously programmed/erased flash rows, 
    // the user must clear the flash cache with the Cy_SysLib_ClearFlashCacheAndBuffer() function.
    Cy_SysLib_ClearFlashCacheAndBuffer();
    
    uint8_t *psrc = (uint8_t *)(addr);
    uint8_t *pdst = (uint8_t *)(dst);
    for (uint32_t i=0; i<size; i++) {
        *(pdst++) = *(psrc++);
    }
    return 0;
}

int ty_hal_write_sector_with_erase(const uint32_t addr, const uint8_t *src, uint32_t size) {
    int ret = 0;
    (void)(size);
    
    if ((addr > CALCULATE_FLASH_ADDRESS(LAST_FLASH_ROW)) || \
        (src == NULL)) 
        return -1;

    /* This is done in three steps - pre-program, erase and then program flash row with the input data */
    if (Cy_Flash_WriteRow((uint32_t)addr, (const uint32_t *)src) != CY_FLASH_DRV_SUCCESS) { 
        log_d("ty hal write_with_erase failed\r\n");
        ret = -1;
    }

    return ret;
}
/* [] END OF FILE */
