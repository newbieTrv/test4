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
#ifndef __TUYA_BLE_APP_OTA_H__
#define __TUYA_BLE_APP_OTA_H__

#include "tuya_ble_type.h"   
#include "ty_hal_flash.h" 
    
#define TUYA_BLE_OTA_VERSION 		    ( 3 )
#define TUYA_BLE_OTA_TYPE    		    ( 0 )    
    
#define OTA_MAX_DATA_LEN                ( 512 ) // Can't over 512bytes
#define OTA_FILE_MD5_LEN                ( 16 ) 
    
#if ( OTA_MAX_DATA_LEN > 512 )
 #error("Error!!! OTA_MAX_DATA_LEN Can't > 512bytes")
#endif 

#define APP_OTA_START_ADDR              ( TY_HAL_FLASH_ZONE_OTA_START_ADDR )
#define APP_OTA_END_ADDR                ( TY_HAL_FLASH_ZONE_OTA_END_ADDR )
#define APP_OTA_FILE_MAX_LEN            ( APP_OTA_END_ADDR - APP_OTA_START_ADDR )

#define APP_OTA_IMAGE_INFO_ADDR         ( TY_HAL_FLASH_ZONE_IMAGE_INFO_ADDR )
#define APP_OTA_DFU_SETTINGS_ADDR       ( TY_HAL_FLASH_ZONE_DFU_SETTINGS_ADDR )
 
#define APP_OTA_STORAGE_PROGRESS_LEN    ( 4*1024 )  
    
typedef enum {
    OTA_REQ_ALLOW = 0,
    OTA_REQ_REFUSE,
} ENUM_OTA_REQ_RESPONSE;

typedef enum {
    OTA_FILE_INFO_RSP_STATE_OK = 0,
    OTA_FILE_INFO_RSP_STATE_PID_ERR,
    OTA_FILE_INFO_RSP_STATE_VER_ERR,
    OTA_FILE_INFO_RSP_STATE_FILE_SIZE_ERR,
} ENUM_OTA_FILE_INFO_RSP_STATE;

typedef enum {
    OTA_DATA_RSP_STATE_OK = 0,
    OTA_DATA_RSP_STATE_PACKET_SEQ_ERR,
    OTA_DATA_RSP_STATE_PACKET_SIZE_ERR,
    OTA_DATA_RSP_STATE_PACKET_CRC_ERR,
    OTA_DATA_RSP_STATE_PACKET_OTHER_ERR,
} ENUM_OTA_DATA_RSP_STATE;

typedef enum {
    OTA_END_RSP_STATE_OK = 0,
    OTA_END_RSP_STATE_FILE_SIZE_ERR,
    OTA_END_RSP_STATE_FILE_CRC_ERR,
    OTA_END_RSP_STATE_OTHER_ERR,
} ENUM_OTA_END_RSP_STATE;

#pragma pack(1) 
typedef struct {
	uint8_t  flag;
	uint8_t  ota_version;
    uint8_t  type;
    uint32_t version;
    uint16_t package_maxlen;
} app_ota_req_rsp_t;
#pragma pack() 

#pragma pack(1) 
typedef struct{
	uint8_t  type;
	uint8_t  pid[8];
    uint32_t version;
    uint8_t  md5[OTA_FILE_MD5_LEN];
    uint32_t file_len;
    uint32_t crc32;
} app_ota_file_info_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint8_t  type;
	uint8_t  state;
    uint32_t old_file_len;
    uint32_t old_crc32;
    uint8_t  old_md5[OTA_FILE_MD5_LEN];
} app_ota_file_info_rsp_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint8_t  type;
    uint32_t offset;
} app_ota_file_offset_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint8_t  type;
    uint32_t offset;
} app_ota_file_offset_rsp_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint8_t  type;
    uint16_t pkg_id;
    uint16_t len;
    uint16_t crc16;
    uint8_t  data[];
} app_ota_data_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint8_t type;
    uint8_t state;
} app_ota_data_rsp_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint8_t  type;
    uint8_t state;
} app_ota_end_rsp_t;
#pragma pack()

#pragma pack(1) 
typedef struct{
	uint32_t len;
    uint32_t crc32;
    uint8_t  md5[OTA_FILE_MD5_LEN];
} app_ota_file_info_storage_t;
#pragma pack() 


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
    uint32_t  crc;                 /**< CRC for the stored DFU settings, not including the CRC itself. If 0xFFFFFFF, the CRC has never been calculated. */
    uint32_t  settings_version;    /**< Version of the current DFU settings struct layout. */
    uint32_t  app_version;         /**< Version of the last stored application. */
    uint32_t  bootloader_version;  /**< Version of the last stored bootloader. */

    dfu_bank_t bank_0;             /**< Bank 0. */
    dfu_bank_t bank_1;             /**< Bank 1. */
    
    uint32_t write_offset;         /**< Write offset for the current operation. */
    uint32_t update_start_address; /**< Size of the SoftDevice. */
} dfu_settings_t;
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

extern void tuya_ble_ota_init(void);
extern void tuya_ble_ota_handler(tuya_ble_ota_data_t *ota);
extern uint32_t ota_disconn_handler(void);

#ifdef __cplusplus
}
#endif

#endif // __TUYA_BLE_APP_OTA_H__

/* [] END OF FILE */
