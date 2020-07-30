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
#include "tuya_ble_app_ota.h"
#include "tuya_ble_app_demo.h"

#include "tuya_ble_type.h"
#include "tuya_ble_api.h"
#include "tuya_ble_log.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_port_psoc6.h"


// 短整型大小端互换
#define BSWAP_16(x)   \
            (uint16_t)((((uint16_t)(x) & 0x00ff) << 8) | \
            (((uint16_t)(x) & 0xff00) >> 8) )
             
// 长整型大小端互换        
#define BSWAP_32(x)   \
            (uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | \
            (((uint32_t)(x) & 0x00ff0000) >> 8) | \
            (((uint32_t)(x) & 0x0000ff00) << 8) | \
            (((uint32_t)(x) & 0x000000ff) << 24)) 
                    
                    
static uint16_t packet_num_expected = 0;        // 接收包序号
static unsigned int recieved_data_crc32 = 0;    // 已接收文件的crc
static uint32_t recieved_file_length;           // 已接收的文件长度
static uint32_t last_length_saved = 0;          // 保存上一次4K进度

static uint8_t row_of_ota_data[TY_HAL_FLASH_SECTOR_SIZE] = {0};
static uint16_t row_of_ota_data_size = 0;
static uint32_t row_of_ota_data_offset = 0;   

//file info
static app_ota_file_info_storage_t old_file_info;
static app_ota_file_info_storage_t new_file_info;
static dfu_settings_t dfu_settings;
static tuya_ble_ota_data_type_t ota_state = TUYA_BLE_OTA_REQ;
static bool ota_success = false;

static uint32_t ota_req_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t ota_file_info_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t ota_file_offset_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t ota_data_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);
static uint32_t ota_end_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp);

void *xOtaRebootTimerHandle = NULL;
static void OtaRebootTimerCallback(void* xTimer);

static void tuya_ble_ota_timer_create(void) {
	tuya_ble_timer_create(&xOtaRebootTimerHandle, 1000u, TUYA_BLE_TIMER_SINGLE_SHOT, OtaRebootTimerCallback);  
	
    if (xOtaRebootTimerHandle == NULL) 
        TUYA_APP_LOG_ERROR("ota reboot timer create failed\r\n");
}

static void tuya_ble_ota_reboot_timer_reset(uint32_t ticks) {
    tuya_ble_timer_restart(xOtaRebootTimerHandle, ticks);
}

static void OtaRebootTimerCallback(void *xTimer) {
    (void)(xTimer);
	ty_hal_system_reboot(); 
}

void tuya_ble_ota_init(void) {
    tuya_ble_ota_timer_create();
    
    packet_num_expected     = 0;
    recieved_file_length    = 0;
    recieved_data_crc32     = 0;
    last_length_saved       = 0;
    memset(&old_file_info, 0x00, sizeof(app_ota_file_info_storage_t));
    memset(&new_file_info, 0x00, sizeof(app_ota_file_info_storage_t));
    
    row_of_ota_data_size    = 0;
    row_of_ota_data_offset  = 0;
    ota_success = false;
    ota_state = TUYA_BLE_OTA_REQ;
}

void tuya_ble_ota_handler(tuya_ble_ota_data_t *ota) {
    tuya_ble_ota_response_t rsp;
    rsp.type = ota->type;
        
    switch (ota->type) {
        case TUYA_BLE_OTA_REQ: 
            ota_req_handler(ota->p_data, ota->data_len, &rsp);
            break;
        
        case TUYA_BLE_OTA_FILE_INFO: 
            ota_file_info_handler(ota->p_data, ota->data_len, &rsp);
            break;
        
        case TUYA_BLE_OTA_FILE_OFFSET_REQ:
            ota_file_offset_handler(ota->p_data, ota->data_len, &rsp);
            break;
        
        case TUYA_BLE_OTA_DATA: 
            ota_data_handler(ota->p_data, ota->data_len, &rsp);
            break;
        
        case TUYA_BLE_OTA_END: 
            ota_end_handler(ota->p_data, ota->data_len, &rsp);
            break;
        
        case TUYA_BLE_OTA_UNKONWN: 
            break;
        
        default:
            // TODO ..
            break;
    }
}

static int ota_erase_sector(uint32_t addr) {
    return ty_hal_flash_erase_sector(addr);
}

static int ota_read_data(uint32_t addr, uint8_t *data, uint32_t len) {
	return ty_hal_flash_read_sector(addr, data, len);
}

static int ota_write_data(uint32_t addr, uint8_t *data, uint32_t len) {
	return ty_hal_write_sector_with_erase(addr, data, len);
}

static uint32_t ota_calc_cumulative_crc32_in_flash(uint32_t image_len) {
    if (image_len == 0)
        return 0xFFFFFFFF;
       
    uint32_t cumulative_crc = 0;
    uint32_t read_addr = APP_OTA_START_ADDR;
    uint32_t total_row = (image_len / TY_HAL_FLASH_SECTOR_SIZE);
    uint32_t remainder = (image_len % TY_HAL_FLASH_SECTOR_SIZE);
    
    for (uint32_t i=0; i<total_row; i++) {
        ota_read_data(read_addr, row_of_ota_data, TY_HAL_FLASH_SECTOR_SIZE);
        cumulative_crc = tuya_ble_crc32_compute(row_of_ota_data, TY_HAL_FLASH_SECTOR_SIZE, &cumulative_crc);
        read_addr += TY_HAL_FLASH_SECTOR_SIZE;
    }

    if (remainder > 0) {
        ota_read_data(read_addr, row_of_ota_data, TY_HAL_FLASH_SECTOR_SIZE);
        cumulative_crc = tuya_ble_crc32_compute(row_of_ota_data, remainder, &cumulative_crc);
        read_addr += remainder;
    }
    
    TUYA_APP_LOG_DEBUG("cumulative_crc->[0x%08X]\r\n", cumulative_crc);
    return cumulative_crc;
}

static uint32_t ota_enter(void) {
    packet_num_expected     = 0;
    recieved_file_length    = 0;
    recieved_data_crc32     = 0;
    last_length_saved       = 0;
    memset(&old_file_info, 0x00, sizeof(app_ota_file_info_storage_t));
    memset(&new_file_info, 0x00, sizeof(app_ota_file_info_storage_t));
    
    row_of_ota_data_size    = 0;
    row_of_ota_data_offset  = 0;
    ota_success = false;
    
    /* Conn param updata request */ 
    extern cy_stc_ble_conn_handle_t ty_bal_get_conn_handle(uint8_t ble_conn_index);
    extern bool ty_bal_conn_param_update(uint8_t bdHandle, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout);
    ty_bal_conn_param_update(ty_bal_get_conn_handle(0).bdHandle, 20, 20, 0, 5000);
    
    return 0;
}

static uint32_t ota_exit(void) {
    if (ota_success) {
        TUYA_APP_LOG_INFO("start reset~~~.");
        
        tuya_ble_ota_reboot_timer_reset(1000);
    } else {
        memset((uint8_t *)&dfu_settings, 0x00, sizeof(dfu_settings_t));
        dfu_settings.crc = tuya_ble_crc32_compute((uint8_t *)&dfu_settings.settings_version, (sizeof(dfu_settings_t)-4), NULL);
        ota_write_data(APP_OTA_DFU_SETTINGS_ADDR, (uint8_t *)&dfu_settings, sizeof(dfu_settings_t));
    }

    ota_state = TUYA_BLE_OTA_REQ;
    return 0;
}

uint32_t get_ota_state(void) {
    return ota_state;
}

uint32_t ota_disconn_handler(void) {
    return ota_exit();
}

static uint32_t ota_response(tuya_ble_ota_response_t* rsp, void* rsp_data, uint16_t data_size) {
    rsp->p_data     = rsp_data;
    rsp->data_len   = data_size;
    return tuya_ble_ota_response(rsp);
}

static uint32_t ota_req_handler(uint8_t *cmd, uint16_t cmd_size, tuya_ble_ota_response_t *rsp) {
    app_ota_req_rsp_t req_rsp;
    memset(&req_rsp, 0x00, sizeof(app_ota_req_rsp_t));
    
    if ((ota_state != TUYA_BLE_OTA_REQ) || \
        (cmd_size != 0x0001) || \
        (*cmd != 0x00)) {
        TUYA_APP_LOG_ERROR("ota req paras err\r\n");
        
        req_rsp.flag = OTA_REQ_REFUSE;
        ota_response(rsp, &req_rsp, sizeof(app_ota_req_rsp_t));
        ota_exit();
        return -1;
    } 
        
    req_rsp.flag            = OTA_REQ_ALLOW;
    req_rsp.ota_version     = TUYA_BLE_OTA_VERSION;
    req_rsp.type            = TUYA_BLE_OTA_TYPE; 
    req_rsp.version         = BSWAP_32(TUYA_DEVICE_FVER_NUM);
    req_rsp.package_maxlen  = BSWAP_16(OTA_MAX_DATA_LEN);
    ota_response(rsp, &req_rsp, sizeof(app_ota_req_rsp_t));
    
    TUYA_APP_LOG_DEBUG("rsp info [%d] [%08X] [%04X]\r\n", req_rsp.flag, req_rsp.version, req_rsp.package_maxlen);
    ota_enter();
    ota_state = TUYA_BLE_OTA_FILE_INFO;
    return 0;
}

static uint32_t ota_file_info_handler(uint8_t *cmd, uint16_t cmd_size, tuya_ble_ota_response_t *rsp) {
    app_ota_file_info_rsp_t file_info_rsp;
    app_ota_file_info_t *file_info = (app_ota_file_info_t*)cmd;
    (void)(cmd_size);
    
    if (ota_state != TUYA_BLE_OTA_FILE_INFO || \
        file_info->type != TUYA_BLE_OTA_TYPE) {
        TUYA_APP_LOG_ERROR("ota file info paras err\r\n");
        ota_exit();
        return -1;
    }

    // 解析文件信息
    file_info->version  = BSWAP_32(file_info->version);
    file_info->file_len = BSWAP_32(file_info->file_len);
    file_info->crc32    = BSWAP_32(file_info->crc32);
    new_file_info.len   = file_info->file_len;
    new_file_info.crc32 = file_info->crc32;
    memcpy(new_file_info.md5, file_info->md5, OTA_FILE_MD5_LEN);
    TUYA_APP_LOG_DEBUG("/**************ota file info***********/\n");
    TUYA_APP_LOG_DEBUG("ota file ver    : ver->[%08x]\r\n", file_info->version);
	TUYA_APP_LOG_DEBUG("ota file length : %d\n", new_file_info.len);
	TUYA_APP_LOG_DEBUG("ota file crc    : 0x%08x\n", new_file_info.crc32);
    
    // rsp
    memset(&file_info_rsp, 0x00, sizeof(app_ota_file_info_rsp_t));
    file_info_rsp.type = file_info->type;   
    if (memcmp(file_info->pid, TUYA_DEVICE_PID, TUYA_BLE_PRODUCT_ID_DEFAULT_LEN)) 
        file_info_rsp.state = OTA_FILE_INFO_RSP_STATE_PID_ERR;
    else if(file_info->version <= TUYA_DEVICE_FVER_NUM) 
        file_info_rsp.state = OTA_FILE_INFO_RSP_STATE_VER_ERR;
    else if(file_info->file_len > APP_OTA_FILE_MAX_LEN) 
        file_info_rsp.state = OTA_FILE_INFO_RSP_STATE_FILE_SIZE_ERR;
    else {
        file_info_rsp.state = OTA_FILE_INFO_RSP_STATE_OK;
        ota_state = TUYA_BLE_OTA_FILE_OFFSET_REQ;
    }
    
    /* Read old file info */
    ota_read_data(APP_OTA_IMAGE_INFO_ADDR, (uint8_t *)&old_file_info, sizeof(app_ota_file_info_storage_t));
	TUYA_APP_LOG_DEBUG("readback old file info, len=[0x%04x], crc=[0x%08x]\n", old_file_info.len, old_file_info.crc32);
    if (memcmp(old_file_info.md5, new_file_info.md5, OTA_FILE_MD5_LEN) != 0) {
        old_file_info.len   = 0;
        old_file_info.crc32 = 0;
        memcpy(old_file_info.md5, new_file_info.md5, OTA_FILE_MD5_LEN);
        ota_write_data(APP_OTA_IMAGE_INFO_ADDR, (uint8_t *)&old_file_info, sizeof(app_ota_file_info_storage_t));
    }
    
    // response
    file_info_rsp.old_file_len  = BSWAP_32(old_file_info.len);
    file_info_rsp.old_crc32     = BSWAP_32(old_file_info.crc32);
    memset(file_info_rsp.old_md5, 0x00, OTA_FILE_MD5_LEN);
    ota_response(rsp, &file_info_rsp, sizeof(app_ota_file_info_rsp_t));
   
    if (file_info_rsp.state != OTA_FILE_INFO_RSP_STATE_OK) {
        TUYA_APP_LOG_ERROR("file info err, state->[%d]\r\n", file_info_rsp.state);
        ota_exit();
    }
    
    return 0;
}

static uint32_t ota_file_offset_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp) {
    app_ota_file_offset_t *file_offset = (app_ota_file_offset_t *)cmd;
    app_ota_file_offset_rsp_t file_offset_rsp;
    uint32_t app_req_file_offset = 0;
    (void)(cmd_size);
    
    if (ota_state != TUYA_BLE_OTA_FILE_OFFSET_REQ || \
        file_offset->type != TUYA_BLE_OTA_TYPE) {
        TUYA_APP_LOG_ERROR("ota file offset paras err\r\n");
        ota_exit();
        return -1;
    }
    
    memset(&file_offset_rsp, 0x00, sizeof(app_ota_file_offset_rsp_t));
    app_req_file_offset = BSWAP_32(file_offset->offset);
    if (app_req_file_offset > 0) {
        ota_read_data(APP_OTA_IMAGE_INFO_ADDR, (uint8_t *)&old_file_info, sizeof(app_ota_file_info_storage_t));
	    TUYA_APP_LOG_DEBUG("old file info, len=[0x%04x], crc=[0x%08x]\n", old_file_info.len, old_file_info.crc32);
        
        recieved_file_length = old_file_info.len;
        recieved_data_crc32  = old_file_info.crc32;
        if ((memcmp(old_file_info.md5, new_file_info.md5, OTA_FILE_MD5_LEN) == 0) && \
            (ota_calc_cumulative_crc32_in_flash(recieved_file_length) == recieved_data_crc32) && \
            (app_req_file_offset >= recieved_file_length) && \
            (recieved_file_length % TY_HAL_FLASH_SECTOR_SIZE) == 0) {
            file_offset_rsp.offset = recieved_file_length;
        } else {
            TUYA_APP_LOG_DEBUG("warning, offset->[%d]-[%d]\r\n", app_req_file_offset, recieved_file_length);

            file_offset_rsp.offset = 0;
            recieved_file_length = 0;
            recieved_data_crc32 = 0;
        }
    }
    
    old_file_info.len   = recieved_file_length;
    old_file_info.crc32 = recieved_data_crc32;
    memcpy(old_file_info.md5, new_file_info.md5, OTA_FILE_MD5_LEN);
    ota_write_data(APP_OTA_IMAGE_INFO_ADDR, (uint8_t *)&old_file_info, sizeof(app_ota_file_info_storage_t));
     
    // Response 
    file_offset_rsp.type    = TUYA_BLE_OTA_TYPE;
    file_offset_rsp.offset  = BSWAP_32(file_offset_rsp.offset);
    ota_response(rsp, &file_offset_rsp, sizeof(app_ota_file_offset_rsp_t));
    
    packet_num_expected     = 0;
    last_length_saved	    = recieved_file_length;
    row_of_ota_data_size    = 0;
    row_of_ota_data_offset  = recieved_file_length;
    ota_state = TUYA_BLE_OTA_DATA;
    return 0;
}

static uint32_t ota_data_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp) {
    app_ota_data_rsp_t ota_data_rsp;
    app_ota_data_t* ota_data = (void *)cmd;

    memset(&ota_data_rsp, 0x00, sizeof(app_ota_data_rsp_t));
    
    if (ota_state != TUYA_BLE_OTA_DATA || \
        ota_data->type != TUYA_BLE_OTA_TYPE) {
        TUYA_APP_LOG_ERROR("ota data paras err\r\n");

        ota_data_rsp.state = OTA_DATA_RSP_STATE_PACKET_OTHER_ERR; 
        ota_response(rsp, &ota_data_rsp, sizeof(app_ota_data_rsp_t));
        ota_exit();
        return -1;
    }
    
    ota_data->pkg_id = BSWAP_16(ota_data->pkg_id);
    ota_data->len    = BSWAP_16(ota_data->len);
    ota_data->crc16  = BSWAP_16(ota_data->crc16);
    TUYA_APP_LOG_DEBUG("ota data info->[%d][%d][0x%04X]\r\n", ota_data->pkg_id, ota_data->len, ota_data->crc16);
    
    ota_data_rsp.type  = TUYA_BLE_OTA_TYPE;
    if (ota_data->pkg_id != packet_num_expected) 
        ota_data_rsp.state = OTA_DATA_RSP_STATE_PACKET_SEQ_ERR;
    else if ((cmd_size-7) != ota_data->len) 
        ota_data_rsp.state = OTA_DATA_RSP_STATE_PACKET_SIZE_ERR;
    else if (tuya_ble_crc16_compute(ota_data->data, ota_data->len, NULL) != ota_data->crc16) 
        ota_data_rsp.state = OTA_DATA_RSP_STATE_PACKET_CRC_ERR;
    else {
        ota_data_rsp.state = OTA_DATA_RSP_STATE_OK;

        recieved_data_crc32 = tuya_ble_crc32_compute((uint8_t *)&ota_data->data, ota_data->len, (uint32_t const *)&recieved_data_crc32);
		//tuya_smartlock_log("Cumulative file crc32->[0x%08x]\r\n", recieved_data_crc32);
        
        memcpy(&row_of_ota_data[row_of_ota_data_size], ota_data->data, ota_data->len);
        row_of_ota_data_size += ota_data->len;
        if (row_of_ota_data_size >= TY_HAL_FLASH_SECTOR_SIZE) {
            row_of_ota_data_size -= TY_HAL_FLASH_SECTOR_SIZE;
            
            TUYA_APP_LOG_DEBUG("write ota data, offset->[%d]\r\n", row_of_ota_data_offset);
            ota_write_data((APP_OTA_START_ADDR + row_of_ota_data_offset), (uint8_t *)&row_of_ota_data, TY_HAL_FLASH_SECTOR_SIZE);
            row_of_ota_data_offset += TY_HAL_FLASH_SECTOR_SIZE;
        } 
    
        packet_num_expected++;
	    recieved_file_length += ota_data->len;
        if (recieved_file_length >= (last_length_saved + APP_OTA_STORAGE_PROGRESS_LEN)) {
			/* 4K保存一次进度 */
            last_length_saved = recieved_file_length;

            old_file_info.len   = recieved_file_length;
            old_file_info.crc32 = recieved_data_crc32;
            ota_write_data(APP_OTA_IMAGE_INFO_ADDR, (uint8_t *)&old_file_info, sizeof(app_ota_file_info_storage_t));
		}  
        
        if (recieved_file_length == new_file_info.len) {
            if (row_of_ota_data_size != 0) 
                ota_write_data((APP_OTA_START_ADDR + row_of_ota_data_offset), (uint8_t *)&row_of_ota_data, TY_HAL_FLASH_SECTOR_SIZE);
         
            ota_state = TUYA_BLE_OTA_END;
        } else if (recieved_file_length < new_file_info.len) 
            ota_state = TUYA_BLE_OTA_DATA;
        else 
            ota_data_rsp.state = OTA_DATA_RSP_STATE_PACKET_OTHER_ERR;
	}
    
    ota_response(rsp, &ota_data_rsp, sizeof(app_ota_data_rsp_t));
    if (ota_data_rsp.state != OTA_DATA_RSP_STATE_OK) {
        TUYA_APP_LOG_ERROR("ota data err state->[%d]\r\n", ota_data_rsp.state);
        ota_exit();
    }
    return 0;
}

static uint32_t ota_end_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp) {
    app_ota_end_rsp_t end_rsp;
    memset(&end_rsp, 0x00, sizeof(app_ota_end_rsp_t));
    end_rsp.type = TUYA_BLE_OTA_TYPE;
    
    if ((ota_state != TUYA_BLE_OTA_END) || \
        (cmd_size != 0x0001) || \
        (*cmd != TUYA_BLE_OTA_TYPE)) {
        end_rsp.state = OTA_END_RSP_STATE_OTHER_ERR;
        ota_response(rsp, &end_rsp, sizeof(app_ota_end_rsp_t));
        ota_exit();
        return -1;
    }
    
    /* Delete old file info */
    old_file_info.len   = 0;
    old_file_info.crc32 = 0;
    memset(old_file_info.md5, 0x00, OTA_FILE_MD5_LEN);
    ota_write_data(APP_OTA_IMAGE_INFO_ADDR, (uint8_t *)&old_file_info, sizeof(app_ota_file_info_storage_t));
            
    if (recieved_file_length != new_file_info.len) 
		end_rsp.state = OTA_END_RSP_STATE_FILE_SIZE_ERR;
	else if (ota_calc_cumulative_crc32_in_flash(recieved_file_length) != new_file_info.crc32) 
		end_rsp.state = OTA_END_RSP_STATE_FILE_CRC_ERR;
	else {
        end_rsp.state = OTA_END_RSP_STATE_OK;
        
        memset((uint8_t *)&dfu_settings, 0x00, sizeof(dfu_settings_t));
        dfu_settings.bank_1.image_size = new_file_info.len;
        dfu_settings.bank_1.image_crc = new_file_info.crc32;
        dfu_settings.bank_1.bank_code = DFU_BANK_VALID_APP;
        dfu_settings.write_offset = 0;
        dfu_settings.update_start_address = APP_OTA_START_ADDR;
        
        dfu_settings.crc = tuya_ble_crc32_compute((uint8_t *)&dfu_settings.settings_version, (sizeof(dfu_settings_t)-4), NULL);
        ota_write_data(APP_OTA_DFU_SETTINGS_ADDR, (uint8_t *)&dfu_settings, sizeof(dfu_settings_t));

        ota_success = true;
        TUYA_APP_LOG_DEBUG("ota success\r\n");
    }
    
    TUYA_APP_LOG_DEBUG("ota end, state->[%d] len->[%d] crc->[%08X]\n", end_rsp.state, recieved_file_length, recieved_data_crc32);
    ota_response(rsp, &end_rsp, sizeof(app_ota_end_rsp_t));

    /* No matter success or failed, exit ota process */
    ota_exit();
    return 0;
}

/* [] END OF FILE */
