#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tuya_ble_port.h"
#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"
#include "aes.h"
#include "md5.h"
#include "hmac.h"

tuya_ble_status_t cy_ble_api_err_code_convert(uint32_t err_code)
{
    tuya_ble_status_t ret;
    
    switch (err_code) {
        case CY_BLE_SUCCESS:
            ret = TUYA_BLE_SUCCESS;
            break;

        default:
            ret = TUYA_BLE_ERR_UNKNOWN;
            break;
    }

    return ret;
}


tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(uint8_t const* p_ad_data, uint8_t ad_len) 
{
    ty_bal_adv_update_adv_data(p_ad_data, ad_len);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(uint8_t const *p_sr_data, uint8_t sr_len) 
{
    ty_bal_adv_update_scan_rsp_data(p_sr_data, sr_len);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_gap_disconnect(void) 
{
    cy_en_ble_api_result_t err = ty_bal_gap_disconnect();
    return cy_ble_api_err_code_convert(err);
}


tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr) 
{
    uint8_t mac[6] = {0};
    ty_bal_get_mac_address(mac);
    
    p_addr->addr_type = TUYA_BLE_ADDRESS_TYPE_PUBLIC;
    memcpy(p_addr->addr, mac, 6);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr) 
{
    uint32_t       err_code;
    cy_stc_ble_gap_bd_addr_t bt_addr;

    bt_addr.type = (p_addr->addr_type == TUYA_BLE_ADDRESS_TYPE_RANDOM) ? (0) : (1);
    memcpy(bt_addr.bdAddr, p_addr->addr, 6);
    
    cy_en_ble_api_result_t err = ty_bal_set_mac_address(&bt_addr);
    return cy_ble_api_err_code_convert(err);
}


tuya_ble_status_t tuya_ble_gatt_send_data(const uint8_t *p_data, uint16_t len) 
{
    uint8_t data_len = len;

    if (data_len > TUYA_BLE_DATA_MTU_MAX) 
	{
        data_len = TUYA_BLE_DATA_MTU_MAX;
    }
    
    cy_en_ble_api_result_t err = ty_bal_gatts_send_notfication(p_data, data_len);
    return cy_ble_api_err_code_convert(err);
}
	

tuya_ble_status_t tuya_ble_common_uart_init(void) 
{
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_common_uart_send_data(const uint8_t *p_data,uint16_t len) 
{  
    ty_pdl_ftm_uart_ops.ftm_uart_send((uint8_t *)p_data, len);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_timer_create(void** p_timer_id,uint32_t timeout_value_ms, tuya_ble_timer_mode mode,tuya_ble_timer_handler_t timeout_handler) 
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    
    if (ty_hal_timer_create(p_timer_id, timeout_value_ms, (app_timer_mode_t)mode, timeout_handler))
        ret = TUYA_BLE_ERR_NO_MEM;
    
    return ret;
}


tuya_ble_status_t tuya_ble_timer_delete(void* timer_id) 
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    
    if (ty_hal_timer_delete(timer_id))
        ret = TUYA_BLE_ERR_INTERNAL;
    
    return ret;
}


tuya_ble_status_t tuya_ble_timer_start(void* timer_id) 
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    
    if (ty_hal_timer_start(timer_id))
        ret = TUYA_BLE_ERR_INTERNAL;
    return ret;
}


tuya_ble_status_t tuya_ble_timer_restart(void* timer_id,uint32_t timeout_value_ms) 
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    
    if (ty_hal_timer_restart(timer_id, timeout_value_ms))
        ret = TUYA_BLE_ERR_INTERNAL;
    
    return ret;
}


tuya_ble_status_t tuya_ble_timer_stop(void* timer_id) 
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

    if (ty_hal_timer_stop(timer_id))
        ret = TUYA_BLE_ERR_INTERNAL;
    
    return ret;
}


void tuya_ble_device_delay_ms(uint32_t ms) 
{
    ty_hal_delay_ms(ms);
}


void tuya_ble_device_delay_us(uint32_t us) 
{
    ty_hal_delay_us(us);
}


tuya_ble_status_t tuya_ble_device_reset(void) 
{
    ty_hal_system_reboot();
    return TUYA_BLE_SUCCESS;
}


void tuya_ble_device_enter_critical(void) 
{   
    ty_hal_system_enter_critical();
}


void tuya_ble_device_exit_critical(void) 
{
    ty_hal_system_exit_critical();
}


tuya_ble_status_t tuya_ble_rtc_get_timestamp(uint32_t *timestamp, int32_t *timezone) 
{
    (void)(timezone);
    *timestamp = ty_hal_get_rtc_time();
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_rtc_set_timestamp(uint32_t timestamp, int32_t timezone) 
{
    ty_hal_set_rtc_time(timestamp, timezone);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_nv_init(void) 
{    
    ty_hal_flash_init();
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_nv_erase(uint32_t addr, uint32_t size) 
{    
    tuya_ble_status_t result = TUYA_BLE_SUCCESS;
    uint32_t erase_pages, i;
    
    /* make sure the start address is a multiple of FLASH_ERASE_MIN_SIZE */
    if (addr % TUYA_NV_ERASE_MIN_SIZE != 0) {
        log_d("the start address is a not multiple of TUYA_NV_ERASE_MIN_SIZE");
        return TUYA_BLE_ERR_INVALID_ADDR;
    }
    
    /* calculate pages */
    erase_pages = size / TUYA_NV_ERASE_MIN_SIZE;
    if (size % TUYA_NV_ERASE_MIN_SIZE != 0) {
        erase_pages++;
    }

    /* start erase */
    for (i = 0; i < erase_pages; i++) {   
        if (ty_hal_flash_erase_sector(addr + (TUYA_NV_ERASE_MIN_SIZE * i))) {
			result = TUYA_BLE_ERR_INTERNAL;
            break;
		}
    }    
    
    return result;
}

tuya_ble_status_t tuya_ble_nv_write(uint32_t addr, const uint8_t *p_data, uint32_t size) 
{  
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    
    if (ty_hal_flash_write_sector(addr, p_data, size))
        ret = TUYA_BLE_ERR_INTERNAL;
    
    return ret;
}

tuya_ble_status_t tuya_ble_nv_read(uint32_t addr,uint8_t *p_data, uint32_t size) 
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

    if (ty_hal_flash_read_sector(addr, p_data, size))
        ret = TUYA_BLE_ERR_INTERNAL;
    
    return ret;
}


#if TUYA_BLE_USE_OS
bool tuya_ble_os_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),void *p_param, uint16_t stack_size, uint16_t priority) 
{
    return ty_hal_os_thread_create(pp_handle, p_name, p_routine, p_param, stack_size, priority);
}

bool tuya_ble_os_task_delete(void *p_handle) 
{
    return ty_hal_os_thread_delete(p_handle);
}

bool tuya_ble_os_task_suspend(void *p_handle) 
{
    return ty_hal_os_thread_suspend(p_handle);
}

bool tuya_ble_os_task_resume(void *p_handle) 
{
    return ty_hal_os_thread_resume(p_handle);
}

bool tuya_ble_os_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size) 
{
    return ty_hal_os_msg_queue_create(pp_handle, msg_num, msg_size);
}

bool tuya_ble_os_msg_queue_delete(void *p_handle) 
{
    return ty_hal_os_msg_queue_delete(p_handle);
}

bool tuya_ble_os_msg_queue_peek(void *p_handle, uint32_t *p_msg_num) 
{
    return ty_hal_os_msg_queue_peek(p_handle, p_msg_num);
}

bool tuya_ble_os_msg_queue_send(void *p_handle, void *p_msg, uint32_t wait_ms) 
{
    return ty_hal_os_msg_queue_send(p_handle, p_msg, wait_ms);
}

bool tuya_ble_os_msg_queue_recv(void *p_handle, void *p_msg, uint32_t wait_ms) 
{
    return ty_hal_os_msg_queue_recv(p_handle, p_msg, wait_ms);
}
#endif // TUYA_BLE_USE_OS


#if TUYA_BLE_USE_PLATFORM_MEMORY_HEAP
void *tuya_ble_port_malloc( uint32_t size ) 
{
    return ty_hal_system_malloc(size);
}

void tuya_ble_port_free( void *pv ) 
{
    ty_hal_system_free(pv);
}
#endif // TUYA_BLE_USE_PLATFORM_MEMORY_HEAP


tuya_ble_status_t tuya_ble_rand_generator(uint8_t* p_buf, uint8_t len)
{
    uint32_t cnt=len/4;
    uint8_t  remain = len%4;
    int32_t temp;
    for(uint32_t i=0; i<cnt; i++)
    {
        temp = rand();
        memcpy(p_buf,(uint8_t *)&temp,4);
        p_buf += 4;
    }
    temp = rand();
    memcpy(p_buf,(uint8_t *)&temp,remain);

    return TUYA_BLE_SUCCESS;
}

bool tuya_ble_aes128_ecb_encrypt(uint8_t *key,uint8_t *input,uint16_t input_len,uint8_t *output) 
{
    uint16_t length;
    mbedtls_aes_context aes_ctx;
    
    if(input_len%16)
    {
        return false;
    }

    length = input_len;

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);

    while( length > 0 )
    {
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, input, output );
        input  += 16;
        output += 16;
        length -= 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return true;
}

bool tuya_ble_aes128_ecb_decrypt(uint8_t *key,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    uint16_t length;
    mbedtls_aes_context aes_ctx;
    
    if(input_len%16)
    {
        return false;
    }

    length = input_len;

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);

    while( length > 0 )
    {
        mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, input, output );
        input  += 16;
        output += 16;
        length -= 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return true;
}

bool tuya_ble_aes128_cbc_encrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output)
{
    mbedtls_aes_context aes_ctx;
    //
    if(input_len%16)
    {
        return false;
    }

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    
    mbedtls_aes_crypt_cbc(&aes_ctx,MBEDTLS_AES_ENCRYPT,input_len,iv,input,output);
    
    mbedtls_aes_free(&aes_ctx);

    return true;
}

bool tuya_ble_aes128_cbc_decrypt(uint8_t *key,uint8_t *iv,uint8_t *input,uint16_t input_len,uint8_t *output) 
{
    mbedtls_aes_context aes_ctx;
    //
    if(input_len%16)
    {
        return false;
    }

    mbedtls_aes_init(&aes_ctx);

    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
    
    mbedtls_aes_crypt_cbc(&aes_ctx,MBEDTLS_AES_DECRYPT,input_len,iv,input,output);
    //
    mbedtls_aes_free(&aes_ctx);

    return true;
}


bool tuya_ble_md5_crypt(uint8_t *input,uint16_t input_len,uint8_t *output) 
{
    mbedtls_md5_context md5_ctx;
    
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, input, input_len);
    mbedtls_md5_finish(&md5_ctx, output);
    mbedtls_md5_free(&md5_ctx);    
    
    return true;
}

bool tuya_ble_hmac_sha1_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output)
{    
    //hmac_sha1_crypt(key, key_len, input, input_len, output);
	return true;
}


bool tuya_ble_hmac_sha256_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output)
{
   // hmac_sha256_crypt(key, key_len, input, input_len, output);
	return true;
}
