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
#ifndef __TUYA_BLE_PSOC63_BOOTLOADER_H__
#define __TUYA_BLE_PSOC63_BOOTLOADER_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// flash 分区
#define TY_HAL_FLASH_ZONE_BOOT_BASE_ROW         ( 0 )    // boot/cm0 size = 192k
#define TY_HAL_FLASH_ZONE_APP0_BASE_ROW         ( 384 )  // app0 size = 320k
#define TY_HAL_FLASH_ZONE_OTA_START_ROW         ( 1024 ) // app1 size = 320k
#define TY_HAL_FLASH_ZONE_OTA_END_ROW           ( 1663 )
#define TY_HAL_FALSH_ZONE_BLE_INFO_BASE_ROW     ( 1664 ) // ble info size = 8k
#define TY_HAL_FLASH_ZONE_DEV_INFO_BASE_ROW     ( 1680 ) // dev info size 
#define TY_HAL_FLASH_ZONE_IMAGE_INFO_ROW        ( 2045 ) // image progress info
#define TY_HAL_FLASH_ZONE_DFU_SETTINGS_ROW      ( 2046 ) // ota dfu info
#define TY_HAL_FLASH_ZONE_END_ROW               ( 2047 ) // last row
        
    
#define LAST_FLASH_ROW                      ( 2047u )    
#define CALCULATE_FLASH_ADDRESS(rowNum)     ( CY_FLASH_BASE + ((rowNum) * CY_FLASH_SIZEOF_ROW) )

#define APP_OTA_START_ADDR                  ( CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_OTA_START_ROW) )
#define APP_OTA_DFU_SETTINGS_ADDR           ( CALCULATE_FLASH_ADDRESS(TY_HAL_FLASH_ZONE_DFU_SETTINGS_ROW) )

/* DFU settings info */
#define DFU_BANK_INVALID         0x00 /**< Invalid image. */
#define DFU_BANK_VALID_APP       0x01 /**< Valid application. */
#define DFU_BANK_VALID_SD        0xA5 /**< Valid SoftDevice. */
#define DFU_BANK_VALID_BL        0xAA /**< Valid bootloader. */
#define DFU_BANK_VALID_SD_BL     0xAC /**< Valid SoftDevice and bootloader. */
#define DFU_BANK_VALID_EXT_APP   0xB1 /**< Valid application designated for a remote node. */

typedef struct {
    uint32_t  image_size;         /**< Size of the image in the bank. */
    uint32_t  image_crc;          /**< CRC of the image. If set to 0, the CRC is ignored. */
    uint32_t  bank_code;          /**< Identifier code for the bank. */
} dfu_bank_t;

#pragma pack(1) 
typedef struct {
    uint32_t  crc;                /**< CRC for the stored DFU settings, not including the CRC itself. If 0xFFFFFFF, the CRC has never been calculated. */
    uint32_t  settings_version;   /**< Version of the current DFU settings struct layout. */
    uint32_t  app_version;        /**< Version of the last stored application. */
    uint32_t  bootloader_version; /**< Version of the last stored bootloader. */

    dfu_bank_t bank_0;             /**< Bank 0. */
    dfu_bank_t bank_1;             /**< Bank 1. */
    
    uint32_t write_offset;         /**< Write offset for the current operation. */
    uint32_t update_start_address; /**< Size of the SoftDevice. */
} dfu_settings_t;
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

extern int ty_hal_flash_erase_sector(const uint32_t addr);
extern int ty_hal_flash_write_sector(const uint32_t addr, const uint8_t *src, uint32_t size);
extern int ty_hal_flash_read_sector(const uint32_t addr, uint8_t *dst, uint32_t size);
extern int ty_hal_write_sector_with_erase(const uint32_t addr, const uint8_t *src, uint32_t size);

extern uint32_t ty_bootloader_firmware_copy(uint32_t firmware_addr, uint32_t firmware_lenth);
bool ty_app_is_valid(dfu_settings_t *dfu_settings);

#ifdef __cplusplus
}
#endif

#endif // __TUYA_BLE_PSOC63_BOOTLOADER_H__

/* [] END OF FILE */
