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
#include "tuya_ble_app_demo.h"

#include "ty_hal_os.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_event.h"
#include "tuya_ble_log.h"
#include "tuya_ble_app_ota.h"
#include "custom_app_product_test.h"

tuya_ble_device_param_t device_param = {0};

// 测试授权码
static const char device_id_test[]  = "zzzzzzzzzzzzzzzz";                  //"zzzzzzzzzzzzzzzz"
static const char auth_key_test[]   = "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";  //"yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy" 
static const uint8_t mac_test[6]    = {0x6A,0x2F,0x12,0x4D,0x23,0xDC};     //The actual MAC address is : 66:55:44:33:22:11

static uint16_t sn = 0;
static uint32_t time_stamp = 1587795793;
static uint8_t dp_data_array[255+3] = {0};
static uint16_t dp_data_len = 0;

/**
 * tuya ble app task
 */
#define APP_TUYA_BLE_TASK_PRIORITY             ( 19u )      //!< Task priorities
#define APP_TUYA_BLE_TASK_STACK_SIZE           ( 512u )     //!< Task stack size
#define MAX_NUMBER_OF_CB_MESSAGE               ( 0x20 )     //!< Message queue size

void *tuya_ble_app_task_handle = NULL;   
void *tuya_ble_app_cb_queue = NULL;
static void tuya_ble_app_task(void *p_param);


/**
 * trace device run state
 */
#define configUSE_TRACE_DEVICE_RUN_STATE    ( 0 )

#if ( configUSE_TRACE_DEVICE_RUN_STATE != 0 ) 
void *xTraceRunStateTimerHandle = NULL;

static void TraceRunStateTimerCallback(void* xTimer);

void tuya_device_trace_run_state_timer_start(void) {
	tuya_ble_timer_create(&xTraceRunStateTimerHandle, 10000u, TUYA_BLE_TIMER_REPEATED, TraceRunStateTimerCallback);  
	tuya_ble_timer_start(xTraceRunStateTimerHandle);
}

static void TraceRunStateTimerCallback(void* xTimer) {
    (void)xTimer;
    
    // YOUR JOBS
}
#endif // ( configUSE_TRACE_DEVICE_RUN_STATE != 0 )      
  

void tuya_ble_app_task_init(void) {
    tuya_ble_os_task_create(&tuya_ble_app_task_handle, \
                        "ble_app_task", \
                        tuya_ble_app_task, \
                        NULL, \
                        APP_TUYA_BLE_TASK_STACK_SIZE, \
                        APP_TUYA_BLE_TASK_PRIORITY);
}   

/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param    Parameters sending to the task
 * @return       void
 */
static void tuya_ble_app_task(void *p_param) {
    (void)(p_param);
    int16_t result = 0;
    tuya_ble_cb_evt_param_t event;
    
    tuya_ble_os_msg_queue_create(&tuya_ble_app_cb_queue, MAX_NUMBER_OF_CB_MESSAGE, sizeof(tuya_ble_cb_evt_param_t));
    ty_hal_os_sleep(500);
    
    // Noticed!!! if use the license stored by the SDK initialized to 0, Otherwise 16 or 20.
    device_param.device_id_len = 16;    
    
    if (device_param.device_id_len == 16) {
        memcpy(device_param.auth_key, auth_key_test, AUTH_KEY_LEN);
        memcpy(device_param.device_id, device_id_test, DEVICE_ID_LEN);
        memcpy(device_param.mac_addr.addr, mac_test, 6);
        device_param.mac_addr.addr_type = TUYA_BLE_ADDRESS_TYPE_RANDOM;
    }
    
    device_param.p_type = TUYA_BLE_PRODUCT_ID_TYPE_PID;
    device_param.product_id_len = 8;
    memcpy(device_param.product_id, TUYA_DEVICE_PID, 8);
    device_param.firmware_version = TUYA_DEVICE_FVER_NUM;
    device_param.hardware_version = TUYA_DEVICE_HVER_NUM;
    
    tuya_ble_sdk_init(&device_param);
    tuya_ble_callback_queue_register(tuya_ble_app_cb_queue);
    
    tuya_ble_ota_init();
    tuya_ble_ftm_init();
    TUYA_APP_LOG_INFO("app version : %s", TUYA_DEVICE_FVER_STR);  
    
    while (true) 
    {
        if (tuya_ble_os_msg_queue_recv(tuya_ble_app_cb_queue, &event, TY_HAL_OS_portMAX_DELAY) == true) {
            switch (event.evt) {
                case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
                    TUYA_APP_LOG_INFO("received tuya ble conncet status update event,current connect status = %d",event.connect_status);
                    break;
                    
                case TUYA_BLE_CB_EVT_DP_WRITE:
                    dp_data_len = event.dp_write_data.data_len;
                    memset(dp_data_array,0,sizeof(dp_data_array));
                    memcpy(dp_data_array,event.dp_write_data.p_data,dp_data_len);
                    TUYA_APP_LOG_INFO("received dp write data :",32,dp_data_array,dp_data_len); // 1

                    sn = 0;
                    tuya_ble_dp_data_report(dp_data_array,dp_data_len);
                    break;
                    
                case TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE:
                    TUYA_APP_LOG_INFO("received dp data report response result code =%d\n",event.dp_response_data.status);
                    break;
                    
                case TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE:
                    TUYA_APP_LOG_INFO("received dp data with time report response result code =%d\n",event.dp_response_data.status);
                    break;
                
                case TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_REPORT_RESPONSE:
                    TUYA_APP_LOG_INFO("received dp data with flag report response sn = %d , flag = %d , result code =%d", \
                                event.dp_with_flag_response_data.sn, \
                                event.dp_with_flag_response_data.mode, \
                                event.dp_with_flag_response_data.status);
                    if (event.dp_with_flag_response_data.mode == REPORT_FOR_CLOUD_PANEL) 
                    {
                        tuya_ble_dp_data_with_flag_report(sn,REPORT_FOR_CLOUD,dp_data_array,dp_data_len);//3
                    }
                    else if (event.dp_with_flag_response_data.mode == REPORT_FOR_CLOUD)
                    {
                        tuya_ble_dp_data_with_flag_report(sn,REPORT_FOR_PANEL,dp_data_array,dp_data_len);//4
                    }
                    else if (event.dp_with_flag_response_data.mode == REPORT_FOR_PANEL)
                    {
                        tuya_ble_dp_data_with_flag_report(sn,REPORT_FOR_NONE,dp_data_array,dp_data_len);//5
                    }
                    else if (event.dp_with_flag_response_data.mode == REPORT_FOR_NONE)
                    {
                        tuya_ble_dp_data_with_flag_and_time_report(sn,REPORT_FOR_CLOUD_PANEL,time_stamp,dp_data_array,dp_data_len);//6
                    }
                    else
                    {
                        
                    }
                    sn++;
                    break;
                    
                case TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORT_RESPONSE:
                    TUYA_APP_LOG_INFO("received dp data with flag and time report response sn = %d , flag = %d , result code =%d", \
                                event.dp_with_flag_and_time_response_data.sn, \
                                event.dp_with_flag_and_time_response_data.mode, \
                                event.dp_with_flag_and_time_response_data.status);
                
                    if (event.dp_with_flag_and_time_response_data.mode == REPORT_FOR_CLOUD_PANEL)
                    {
                        tuya_ble_dp_data_with_flag_and_time_report(sn,REPORT_FOR_CLOUD,time_stamp,dp_data_array,dp_data_len);//7
                    }
                    else if (event.dp_with_flag_and_time_response_data.mode == REPORT_FOR_CLOUD)
                    {
                        tuya_ble_dp_data_with_flag_and_time_report(sn,REPORT_FOR_PANEL,time_stamp,dp_data_array,dp_data_len);//8
                    }
                    else if (event.dp_with_flag_and_time_response_data.mode == REPORT_FOR_PANEL)
                    {
                        tuya_ble_dp_data_with_flag_and_time_report(sn,REPORT_FOR_NONE,time_stamp,dp_data_array,dp_data_len); //9
                    }
                    else
                    {
                        
                    }
                    sn++;
                    break;
                    
                case TUYA_BLE_CB_EVT_UNBOUND:
                    TUYA_APP_LOG_INFO("received unbound req\n");
                    break;
                    
                case TUYA_BLE_CB_EVT_ANOMALY_UNBOUND:
                    TUYA_APP_LOG_INFO("received anomaly unbound req\n");
                    break;
                    
                case TUYA_BLE_CB_EVT_DEVICE_RESET:
                    TUYA_APP_LOG_INFO("received device reset req\n");
                    break;
                   
                case TUYA_BLE_CB_EVT_DP_QUERY:
                    TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_DP_QUERY event");
                    tuya_ble_dp_data_report(dp_data_array,dp_data_len);
                    break;
                    
                case TUYA_BLE_CB_EVT_OTA_DATA:
                    TUYA_APP_LOG_INFO("received TUYA_BLE_CB_EVT_OTA_DATA event");
                    tuya_ble_ota_handler(&event.ota_data);
                    break;
                    
                case TUYA_BLE_CB_EVT_NETWORK_INFO:
                    TUYA_APP_LOG_INFO("received net info : %s", event.network_data.p_data);
                    tuya_ble_net_config_response(result);
                    break;
                    
                case TUYA_BLE_CB_EVT_WIFI_SSID:
                    break;
                    
                case TUYA_BLE_CB_EVT_TIME_STAMP:
                    TUYA_APP_LOG_INFO("received unix timestamp : %s ,time_zone : %d",event.timestamp_data.timestamp_string,event.timestamp_data.time_zone);
                    break;
                    
                case TUYA_BLE_CB_EVT_TIME_NORMAL:
                    break;
                    
                case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:
                    TUYA_APP_LOG_HEXDUMP_DEBUG("received ble passthrough data :",event.ble_passthrough_data.p_data,event.ble_passthrough_data.data_len);
                    tuya_ble_data_passthrough(event.ble_passthrough_data.p_data,event.ble_passthrough_data.data_len);
                    break;
                    
                default:
                    TUYA_APP_LOG_WARNING("app_tuya_cb_queue msg: unknown event type 0x%04x",event.evt);
                    break;
            }

            tuya_ble_event_response(&event);
        }
    }
}

/* [] END OF FILE */

