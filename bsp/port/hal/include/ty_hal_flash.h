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
#ifndef __TY_HAL_FLASH_H__
#define __TY_HAL_FLASH_H__
    
#include <stdint.h>
#include "cy_flash.h"

/* Check TRM details to get this number. */
#define LAST_FLASH_ROW                      ( 2047u )

/* This array reserves space in the flash for one row of size
 * CY_FLASH_SIZEOF_ROW. Explicit initialization is required so that memory is
 * allocated in flash instead of RAM. */
#define CALCULATE_FLASH_ADDRESS(rowNum)     ( CY_FLASH_BASE + ((rowNum) * CY_FLASH_SIZEOF_ROW) )

// 页大小
#define TY_HAL_FLASH_SECTOR_SIZE            ( CY_FLASH_SIZEOF_ROW )

// flash分区
#define TY_HAL_FLASH_ZONE_BOOT_BASE_ROW         ( 0 )    // boot/cm0 size = 192k
#define TY_HAL_FLASH_ZONE_APP0_BASE_ROW         ( 384 )  // app0 size = 320k
#define TY_HAL_FLASH_ZONE_OTA_START_ROW         ( 1024 ) // app1 size = 320k
#define TY_HAL_FLASH_ZONE_OTA_END_ROW           ( 1663 )
#define TY_HAL_FALSH_ZONE_BLE_INFO_BASE_ROW     ( 1664 ) // ble info size = 8k
#define TY_HAL_FLASH_ZONE_DEV_INFO_BASE_ROW     ( 1680 ) // dev info size 
#define TY_HAL_FLASH_ZONE_IMAGE_INFO_ROW        ( 2045 ) // image progress info
#define TY_HAL_FLASH_ZONE_DFU_SETTINGS_ROW      ( 2046 ) // ota dfu settings
#define TY_HAL_FLASH_ZONE_END_ROW               ( 2047 ) // last row
        
#define TY_HAL_FLASH_ZONE_OTA_START_ADDR        (CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_OTA_START_ROW))
#define TY_HAL_FLASH_ZONE_OTA_END_ADDR          (CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_OTA_END_ROW))
#define TY_HAL_FLASH_ZONE_BLE_INFO_BASE_ADDR    (CALCULATE_FLASH_ADDRESS(TY_HAL_FALSH_ZONE_BLE_INFO_BASE_ROW))
#define TY_HAL_FLASH_ZONE_DEV_INFO_BASE_ADDR    (CALCULATE_FLASH_ADDRESS(TY_HAL_FALSH_ZONE_DEV_INFO_BASE_ROW))
#define TY_HAL_FLASH_ZONE_IMAGE_INFO_ADDR       (CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_IMAGE_INFO_ROW))
#define TY_HAL_FLASH_ZONE_DFU_SETTINGS_ADDR     (CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_DFU_SETTINGS_ROW))
#define TY_HAL_FLASH_ZONE_END_ADDR              (CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_END_ROW))

#ifdef __cplusplus
 extern "C" {
#endif 
    
extern void ty_hal_flash_init(void);
extern int ty_hal_flash_erase_sector(const uint32_t addr);
extern int ty_hal_flash_write_sector(const uint32_t addr, const uint8_t *src, uint32_t size);
extern int ty_hal_flash_read_sector(const uint32_t addr, uint8_t *dst, uint32_t size);
extern int ty_hal_write_sector_with_erase(const uint32_t addr, const uint8_t *src, uint32_t size);

#ifdef __cplusplus
}
#endif 

#endif // __TY_HAL_FLASH_H__

/* [] END OF FILE */