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
#include "tuya_ble_psoc63_bootloader.h"
#include "uart_debug.h"

void ty_hal_flash_init(void) {
    // This function is called in the SystemInit() function,
    //Cy_Flash_Init();
}

int ty_hal_flash_erase_sector(const uint32_t addr) {
    int ret = 0;
    
    if (Cy_Flash_EraseRow(addr) != CY_FLASH_DRV_SUCCESS) {
        DebugPrintf("ty hal flash erase failed\r\n");
        ret = -1;
    }
   
    return ret;
}

int ty_hal_flash_write_sector(const uint32_t addr, const uint8_t *src, uint32_t size) {
    int ret = 0;
    
    if ((addr > CALCULATE_FLASH_ADDRESS(LAST_FLASH_ROW)) || \
        (src == NULL) || 
        (size > CY_FLASH_SIZEOF_ROW)) 
        return -1;
   
    /* This is done in three steps - pre-program, erase and then program flash row with the input data */
    if (Cy_Flash_ProgramRow((uint32_t)addr, (const uint32_t *)src) != CY_FLASH_DRV_SUCCESS) { 
        DebugPrintf("ty hal flash write failed\r\n");
        ret = -1;
    }
    
    return ret;
}

int ty_hal_flash_read_sector(const uint32_t addr, uint8_t *dst, uint32_t size) {
    if ((addr > CALCULATE_FLASH_ADDRESS(LAST_FLASH_ROW)) || (dst == NULL)) {
        DebugPrintf("ty hal flash read failed\r\n");
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
        DebugPrintf("ty hal write_with_erase failed\r\n");
        ret = -1;
    }

    return ret;
}

uint32_t tuya_ble_crc32_compute(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc) {
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    
    for (uint32_t i = 0; i < size; i++) {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--) {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}

uint32_t ty_bootloader_firmware_copy(uint32_t firmware_addr, uint32_t firmware_lenth) {
    uint8_t buf[CY_FLASH_SIZEOF_ROW] = {0};
    uint32_t read_addr = 0;
    uint32_t total_rows = (firmware_lenth / CY_FLASH_SIZEOF_ROW);
    uint32_t remainder = (firmware_lenth % CY_FLASH_SIZEOF_ROW);
    
    DebugPrintf("copying image, fw_adrr->[0x%08"PRIX32"] fw_size->[%"PRIu32"]\n", firmware_addr, firmware_lenth);
    for (uint32_t i=0; i<total_rows; i++) {
        ty_hal_flash_read_sector((firmware_addr + read_addr), buf, CY_FLASH_SIZEOF_ROW);
        ty_hal_write_sector_with_erase(CY_CORTEX_M4_APPL_ADDR+read_addr, buf, CY_FLASH_SIZEOF_ROW);

        read_addr += CY_FLASH_SIZEOF_ROW;
    }

    if (remainder > 0) {
        ty_hal_flash_read_sector((firmware_addr + read_addr), buf, CY_FLASH_SIZEOF_ROW);
        ty_hal_write_sector_with_erase(CY_CORTEX_M4_APPL_ADDR+read_addr, buf, CY_FLASH_SIZEOF_ROW);
    }
    
    return 0;
}

bool ty_app_is_valid(dfu_settings_t *dfu_settings) {
    uint8_t buf[CY_FLASH_SIZEOF_ROW] = {0};
    uint32_t cumulative_crc = 0;
    uint32_t read_addr, total_row, remainder;
    bool is_valid = false;
    
    /* Check settings crc */
    uint32_t dfu_settings_crc = tuya_ble_crc32_compute((uint8_t *)&(dfu_settings->settings_version), (sizeof(dfu_settings_t)-4), NULL);
    if (dfu_settings_crc != dfu_settings->crc) {
        DebugPrintf("dfu settings crc->[%"PRIx32"]-[%"PRIx32"]\r\n", dfu_settings_crc, dfu_settings->crc);
        return false;
    }
        
    /* Check image crc32 */
    if (dfu_settings->update_start_address == APP_OTA_START_ADDR && \
        dfu_settings->bank_1.bank_code == DFU_BANK_VALID_APP && \
        dfu_settings->bank_1.image_size != 0) {
        read_addr = dfu_settings->update_start_address;
        total_row = (dfu_settings->bank_1.image_size / CY_FLASH_SIZEOF_ROW);
        remainder = (dfu_settings->bank_1.image_size % CY_FLASH_SIZEOF_ROW);
        
        for (uint32_t i=0; i<total_row; i++) {
            ty_hal_flash_read_sector(read_addr, buf, CY_FLASH_SIZEOF_ROW);
            cumulative_crc = tuya_ble_crc32_compute(buf, CY_FLASH_SIZEOF_ROW, &cumulative_crc);
            read_addr += CY_FLASH_SIZEOF_ROW;
        }

        if (remainder > 0) {
            ty_hal_flash_read_sector(read_addr, buf, CY_FLASH_SIZEOF_ROW);
            cumulative_crc = tuya_ble_crc32_compute(buf, remainder, &cumulative_crc);
            read_addr += remainder;
        } 
        
        DebugPrintf("cumulative_crc->[%08"PRIX32"]-[%08"PRIX32"]\r\n", cumulative_crc, dfu_settings->bank_1.image_crc);
        if (cumulative_crc == dfu_settings->bank_1.image_crc) {
            is_valid = true;
        }
    }
    
    return is_valid;
}

/* [] END OF FILE */
