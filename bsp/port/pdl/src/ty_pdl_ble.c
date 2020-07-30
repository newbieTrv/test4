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
#include "ty_pdl_ble.h"
#include "tuya_ble_api.h"


/* Task Handle */
void *ble_pdl_task_handle = NULL;
#define BLE_PDL_TASK_PRIORITY           ( 20u ) 
#define BLE_PDL_TASK_STACK_SIZE         ( 512u )

/* Queue Handle */
void *ble_queue_handle = NULL;
#define BLE_COMMAND_QUEUE_LEN           ( 16u )

/* Variable used to maintain connection information */
static cy_stc_ble_conn_handle_t appConnHandle[CY_BLE_CONN_COUNT];

/* tuya spp services handle */
#define TUYA_SPP_COMMON_CCCD_HANDLE     ( CY_BLE_TUYA_SPP_COMMON_SPP_COMMON_TX_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE )
#define TUYA_SPP_COMMON_RX_HANDLE       ( CY_BLE_TUYA_SPP_COMMON_SPP_COMMON_RX_CHAR_HANDLE )


/** 
 * These static functions are used by the BLE Task. These are not available 
 * outside this file. See the respective function definitions for more 
 * details. 
 */
static void ble_pdl_task(void* pvParameters);
static void BleControllerInterruptEventHandler(void);
static void StackEventHandler(uint32_t eventType, void *eventParam);
static void StartAdvertisement(void);
static void UpdateGattDB(cy_stc_ble_gatts_write_cmd_req_param_t writeReqParam);


/* Timer handles */
#define BLE_TIMER_NUMS                          ( 2 )
#define BLE_TIMER_ID_CONN_PARAM_UPDATA          ( 0 )
#define BLE_TIMER_ID_DATALINK_SILENCE_TIMEOUT   ( 1 )

static void xBleTimerCallback_updata_conn_params(void *xTimer);
static void xBleTimerCallback_datalink_silence_timeout(void *xTimer);

void *xBleTimerHandle[BLE_TIMER_NUMS] = {NULL};
static xTimeParas xBleTimer[BLE_TIMER_NUMS] = {
    {(3000),      TimerOnce,     0,     "",        xBleTimerCallback_updata_conn_params},
    {(120000),    TimerOnce,     1,     "",        xBleTimerCallback_datalink_silence_timeout}, 
};

static void create_ble_related_timer(void) {
    /* Create an RTOS timer */
    for (int id=0; id<BLE_TIMER_NUMS; id++) {
        ty_hal_timer_create(&xBleTimerHandle[id], \
							xBleTimer[id].period, \
							xBleTimer[id].type, \
							xBleTimer[id].cb); 
        
        if (xBleTimerHandle[id] == NULL) 
            log_d("create ble[pdl] timer failed, timer id->[%d]\n", id);
    }  
}  

/*
 * Master连接后，请求更新连接参数
*/
static void ble_conn_param_updata_timer_reset(void) {
    int id = BLE_TIMER_ID_CONN_PARAM_UPDATA;
    ty_hal_timer_restart(xBleTimerHandle[id], xBleTimer[id].period);
}

static void ble_conn_param_updata_timer_stop(void) {
    int id = BLE_TIMER_ID_CONN_PARAM_UPDATA;
    ty_hal_timer_stop(xBleTimerHandle[id]);
}

static void xBleTimerCallback_updata_conn_params(void *xTimer) {
    (void)(xTimer);
    ble_msg_t ble_msg = {.command = CONN_PARAM_UPDATA};
    ty_hal_os_msg_queue_send(ble_queue_handle, &ble_msg, 0);
}

/*
 * ble链路静默超时, 可用于连接后无数据交互断开当前连接
*/
static void ble_datalink_silence_timer_reset(uint32_t ticks) {
    int id = BLE_TIMER_ID_DATALINK_SILENCE_TIMEOUT;
    ty_hal_timer_restart(xBleTimerHandle[id], ticks);
}

static void ble_datalink_silence_timeout_timer_stop(void) {
    int id = BLE_TIMER_ID_DATALINK_SILENCE_TIMEOUT;
    ty_hal_timer_stop(xBleTimerHandle[id]);
}

static void xBleTimerCallback_datalink_silence_timeout(void *xTimer) {
    (void)(xTimer);
    // TODO ..
}


/*
 * ty_bal_set_mac_address 设置mac地址
*/
cy_en_ble_api_result_t ty_bal_set_mac_address(cy_stc_ble_gap_bd_addr_t *p_addr) {
    memcpy(cy_ble_config.deviceAddress->bdAddr, p_addr->bdAddr, CY_BLE_BD_ADDR_SIZE);
    
    cy_en_ble_api_result_t api_result = Cy_BLE_GAP_SetBdAddress(p_addr);
    Cy_BLE_ProcessEvents();
    
    log_d("ty bal set mac api-> [%x]\r\n", api_result);
    return api_result;
}


/*
 * ty_bal_get_mac_address 获取mac地址
*/
cy_en_ble_api_result_t ty_bal_get_mac_address(uint8_t *mac) { 
    cy_en_ble_api_result_t api_result = CY_BLE_SUCCESS;

    memcpy(mac, cy_ble_config.deviceAddress->bdAddr, CY_BLE_BD_ADDR_SIZE);
    //elog_hexdump("ble bd addr ", 16, cy_ble_config.deviceAddress->bdAddr, CY_BLE_BD_ADDR_SIZE);
    
    #if 0
    api_result = Cy_BLE_GAP_GetBdAddress();
    Cy_BLE_ProcessEvents();
    log_d("ty bal get mac api return: %d\r\n", api_result);
    #endif
    
    return api_result;
}


/*
 * ty_bal_get_conn_handle 获取连接句柄
*/
cy_stc_ble_conn_handle_t ty_bal_get_conn_handle(uint8_t ble_conn_index) {
    return appConnHandle[ble_conn_index]; 
}


/*
 * ty_bal_get_conn_stu 获取连接状态
*/
bool ty_bal_get_conn_stu(void) {
    cy_stc_ble_conn_handle_t m_conn_handle = ty_bal_get_conn_handle(0);
    
    return (Cy_BLE_GetConnectionState(m_conn_handle) == CY_BLE_CONN_STATE_CONNECTED);
}


/*
 * ty_bal_advertising_init 广播初始化 
*/
void ty_bal_advertising_init(void) {
    // TODO ..
}


/*
 * ty_bal_advertising_start 启动广播 
*/
void ty_bal_advertising_start(void) {
    ble_msg_t ble_msg = {.command = START_BLE_ADV};
    ty_hal_os_msg_queue_send(ble_queue_handle, &ble_msg, 0);
}


/*
 * ty_bal_advertising_stop 停止广播 
*/
void ty_bal_advertising_stop(void) {
    cy_stc_ble_conn_handle_t m_conn_handle = ty_bal_get_conn_handle(0);
    if (Cy_BLE_GetConnectionState(m_conn_handle) == CY_BLE_CONN_STATE_CONNECTED) {
        log_d("stop adv, first disconnect\r\n");
        ty_bal_gap_disconnect();
    }
    
    if (Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
        return;
    
    ble_msg_t ble_msg = {.command = STOP_BLE_ADV};
    ty_hal_os_msg_queue_send(ble_queue_handle, &ble_msg, 0);
}


/*
 * ty_bal_scan_start 启动扫描
*/
void ty_bal_scan_start(void) {
    ble_msg_t ble_msg = {.command = START_BLE_SCAN};
    ty_hal_os_msg_queue_send(ble_queue_handle, &ble_msg, 0);
}


/*
 * ty_bal_scan_stop 停止扫描
*/
void ty_bal_scan_stop(void) {
    ble_msg_t ble_msg = {.command = STOP_BLE_SCAN};
    ty_hal_os_msg_queue_send(ble_queue_handle, &ble_msg, 0);
}


/*
 * ty_bal_adv_update_adv_data 更新adv数据
*/
bool ty_bal_adv_update_adv_data(uint8_t const* p_adv_data, uint8_t adv_len) {
    cy_en_ble_api_result_t api_result = CY_BLE_SUCCESS;
    
    memcpy(&cy_ble_config.discoveryModeInfo->advData->advData, p_adv_data, adv_len);
    cy_ble_config.discoveryModeInfo->advData->advDataLen = adv_len;
    
    #if 0
    if (Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) {
        api_result = Cy_BLE_GAPP_UpdateAdvScanData(&cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex]);
        Cy_BLE_ProcessEvents();
    }
    #endif 
    
    log_d("ty_bal_adv_update_adv_data ->[%x]\r\n", api_result);
    return true;
}


/*
 * ty_bal_adv_update_scan_rsp_data 更新ScanRsp数据
*/
bool ty_bal_adv_update_scan_rsp_data(uint8_t const *p_sr_data, uint8_t sr_len) {
    cy_en_ble_api_result_t api_result = CY_BLE_SUCCESS;
    
    memcpy(&cy_ble_config.discoveryModeInfo->scanRspData->scanRspData, p_sr_data, sr_len);
    cy_ble_config.discoveryModeInfo->scanRspData->scanRspDataLen = sr_len;
    
    if (Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) {
        api_result = Cy_BLE_GAPP_UpdateAdvScanData(&cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex]);
        Cy_BLE_ProcessEvents();
    }
    
    log_d("ty_bal_adv_update_scan_rsp_data ->[%x]\r\n", api_result);
    return true;
}
 

/*
 * ty_bal_set_adv_interval_only 仅设置广播间隔 
*/
static void ty_bal_set_adv_interval_only(uint32_t adv_inv /* unit:ms */) {
	uint16_t ble_adv_interval = (uint16_t)(adv_inv*BLE_APP_ADVINTERVAL_RES);
        
    cy_ble_config.gappAdvParams[0].fastAdvIntervalMax   = ble_adv_interval;  
    cy_ble_config.gappAdvParams[0].fastAdvIntervalMin   = ble_adv_interval; 
    cy_ble_config.gappAdvParams[0].fastAdvTimeOut       = 0;
    cy_ble_config.gappAdvParams[0].slowAdvEnable        = 0; 
}

/*
 * ty_bal_set_adv_interval_immediately_effective 设置广播间隔立即生效
*/
void ty_bal_set_adv_interval_immediately_effective(uint32_t adv_inv /* unit:ms */) {
    Cy_BLE_GAPP_StopAdvertisement();  

    /* wait until advertising stops */
    while(Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_STOPPED) {
        Cy_BLE_ProcessEvents();
    }    
    
    ty_bal_set_adv_interval_only(adv_inv);
    
    StartAdvertisement();
    Cy_BLE_ProcessEvents();
}

/*
 * ty_bal_set_adv_interval 设置广播周期 根据当前状态自动处理
*/
void ty_bal_set_adv_interval(uint32_t adv_inv /* unit:ms */) {
    if (Cy_BLE_GetConnectionState(ty_bal_get_conn_handle(0)) == CY_BLE_CONN_STATE_CONNECTED) {
        log_d("ty_bal_set_adv_interval, current is connect\r\n");
		ty_bal_set_adv_interval_only(adv_inv);
		return;
	}
    
    if (Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_STOPPED) {
        log_d("ty_bal_set_adv_interval, current adv start, stop first\r\n");
        Cy_BLE_GAPP_StopAdvertisement();  
        
        /* wait until advertising stops */
        while(Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_STOPPED) {
            Cy_BLE_ProcessEvents();
        }    
    }
    
    ty_bal_set_adv_interval_only(adv_inv);
    if (adv_inv != 0) {
		StartAdvertisement();
        Cy_BLE_ProcessEvents();
	} 
    
    log_d("ty_bal_set_adv_interval, inv->[%d]", adv_inv);
}


/*
 * ty_bal_conn_param_update 连接参数更新请求
*/
bool ty_bal_conn_param_update(uint8_t bdHandle, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout) {
    cy_stc_ble_gap_conn_update_param_info_t connUpdateParam = {
        .connIntvMin   = MSEC_TO_UNITS(cMin, UNIT_1_25_MS),
        .connIntvMax   = MSEC_TO_UNITS(cMax, UNIT_1_25_MS),
        .connLatency   = latency,
        .supervisionTO = MSEC_TO_UNITS(timeout, UNIT_10_MS),
        .bdHandle      = bdHandle,
    };

    cy_en_ble_api_result_t api_result = Cy_BLE_L2CAP_LeConnectionParamUpdateRequest(&connUpdateParam);
    Cy_BLE_ProcessEvents();
    
    log_d("ty_bal_conn_param_update->[%d]\r\n", api_result);
    return api_result;
}


/*
 * ty_bal_mtu_exchange_req 请求MTU交换，适用于center角色
*/
bool ty_bal_mtu_exchange_req(uint16_t gatt_mtu) {
    cy_stc_ble_gatt_xchg_mtu_param_t param;
    
    param.connHandle = ty_bal_get_conn_handle(0);
    param.mtu = gatt_mtu;
    
    cy_en_ble_api_result_t api_result = Cy_BLE_GATTC_ExchangeMtuReq(&param);
    Cy_BLE_ProcessEvents();
    
    log_d("ty_bal_mtu_exchange_req->[%d]\r\n", api_result);
    return (api_result == CY_BLE_SUCCESS) ? (true) : (false);
}


/*
 * ty_bal_mtu_exchange_rsp 请求MTU交换响应
*/
bool ty_bal_mtu_exchange_rsp(uint16_t gatt_mtu) {
    cy_stc_ble_gatt_xchg_mtu_param_t param;
    
    param.connHandle = ty_bal_get_conn_handle(0);
    param.mtu = gatt_mtu;
    cy_en_ble_api_result_t api_result = Cy_BLE_GATTS_ExchangeMtuRsp(&param);
    Cy_BLE_ProcessEvents();
    
    log_d("ty_bal_mtu_exchange_rsp->[%d]\r\n", api_result);
    return (api_result == CY_BLE_SUCCESS) ? (true) : (false);
}


/*
 * ty_bal_gatts_send_notfication GATT发送数据
*/
cy_en_ble_api_result_t ty_bal_gatts_send_notfication(const uint8_t *data, uint16_t length) {
    if (data == NULL || length == 0)
        return CY_BLE_ERROR_INVALID_PARAMETER;
    
    cy_en_ble_api_result_t api_result = CY_BLE_ERROR_NO_CONNECTION;
    cy_stc_ble_conn_handle_t m_conn_handle = ty_bal_get_conn_handle(0);
    
    if ((Cy_BLE_GetConnectionState(m_conn_handle) ==  CY_BLE_CONN_STATE_CONNECTED ) &&
        (Cy_BLE_GATTS_IsNotificationEnabled(&m_conn_handle, TUYA_SPP_COMMON_CCCD_HANDLE) == true)) {
        /*  Local variable used for storing notification parameter */
        cy_stc_ble_gatts_handle_value_ntf_t customNotificationHandle = {
            .connHandle = m_conn_handle,
            .handleValPair.attrHandle   = CY_BLE_TUYA_SPP_COMMON_SPP_COMMON_TX_CHAR_HANDLE,
            .handleValPair.value.val    = (uint8_t *)data,
            .handleValPair.value.len    = length,
        };
        
        /* Check GATT status */
        while(Cy_BLE_GATT_GetBusyStatus(customNotificationHandle.connHandle.attId) != CY_BLE_STACK_STATE_FREE) {
            Cy_BLE_ProcessEvents();
        }
        
        //elog_hexdump("ble spp comm snd notfiy", 32, (uint8_t *)data, length);
        api_result = Cy_BLE_GATTS_Notification(&customNotificationHandle);
    } 
    
    if (api_result != CY_BLE_SUCCESS && \
        api_result != CY_BLE_ERROR_NO_CONNECTION)
        log_d("GATT Notification err, code->[%d]\r\n", api_result);
    
    return api_result;
}


/*
 * ty_bal_gap_disconnect 断开连接
*/
cy_en_ble_api_result_t ty_bal_gap_disconnect(void) {
    cy_stc_ble_conn_handle_t m_conn_handle = ty_bal_get_conn_handle(0);
    if (Cy_BLE_GetConnectionState(m_conn_handle) != CY_BLE_CONN_STATE_CONNECTED){
       log_d("ty_bal_gap_disconnect, already disconnect\r\n");
       return CY_BLE_SUCCESS;
    }
    
    cy_stc_ble_gap_disconnect_info_t param = {
        /** bd handle of the remote device */
        .bdHandle = ty_bal_get_conn_handle(0).bdHandle, 
        /** Reason for disconnection. */
        .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER,
    };
    
    cy_en_ble_api_result_t ret = Cy_BLE_GAP_Disconnect(&param);  
    Cy_BLE_ProcessEvents();
    
    #if 0
    /* wait until disconnect */
    while (Cy_BLE_GetConnectionState(ty_bal_get_conn_handle(0)) == CY_BLE_CONN_STATE_CONNECTED) {
        Cy_BLE_ProcessEvents();
    }
    #endif 
    
    log_d("gap disconnect-[%x]\r\n", ret);
    return ret;
}


/**
 * @brief  Initialize ble task
 * @return void
 */
void ble_pdl_task_init(void) {
   ty_hal_os_thread_create(&ble_pdl_task_handle, \
            "ble task", \
            ble_pdl_task, \
            NULL, \
            BLE_PDL_TASK_STACK_SIZE, BLE_PDL_TASK_PRIORITY);
}
      
/*******************************************************************************
* Function Name: void ble_pdl_task(void *pvParameters)
********************************************************************************
* Summary:
*  Task that processes the BLE state and events, and then commands other tasks 
*  to take an action based on the current BLE state and data received over BLE  
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  None
*
*******************************************************************************/
static void ble_pdl_task(void* pvParameters) {
    /* Remove warning for unused parameter */
    (void)pvParameters;
    
    BaseType_t rtosApiResult;
    
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    cy_stc_ble_stack_lib_version_t  stackVersion;  
    
    /* Variable that stores BLE commands that need to be processed */
    ble_msg_t bleCommandData;
    
    /* Create an RTOS Queue */
    ty_hal_os_msg_queue_create(&ble_queue_handle, BLE_COMMAND_QUEUE_LEN, sizeof(ble_msg_t));
    
    /* Create an RTOS timer. */
    create_ble_related_timer();
            
    bleApiResult = Cy_BLE_GetStackLibraryVersion(&stackVersion);
    if (bleApiResult != CY_BLE_SUCCESS) {
        log_d("Cy_BLE_GetStackLibraryVersion API Error: 0x%2.2x \r\n", bleApiResult);
    } else {
        log_d("Stack Library Version: [%u.%u.%u.%u]\r\n", \
                stackVersion.majorVersion, \
                stackVersion.minorVersion, \
                stackVersion.patch, \
                stackVersion.buildNumber);
    }
    
    /* Start the BLE component and register the stack event handler */
    bleApiResult = Cy_BLE_Start(StackEventHandler);
    
    /* Check if the operation was successful */
    if (bleApiResult == CY_BLE_SUCCESS) {
        log_d("Success  : BLE - Stack initialization");
    
        /* Register the application Host callback */
        Cy_BLE_RegisterAppHostCallback(BleControllerInterruptEventHandler);

        /* Process BLE events to turn the stack on */
        Cy_BLE_ProcessEvents();
    } else {
        log_d("Failure! : BLE  - Stack initialization. Error Code:", bleApiResult);
    }
    
    /* Repeatedly running part of the task */
    for (;;)
    {
        /* Block until a BLE command has been received over ble_queue */
        if (ty_hal_os_msg_queue_recv(ble_queue_handle, &bleCommandData, TY_HAL_OS_portMAX_DELAY) == true) {
            switch (bleCommandData.command) {
                /*~~~~~~~~~~~~~~ Command to process BLE events ~~~~~~~~~~~~~~~*/
                case PROCESS_BLE_EVENTS:
                    /* Process event callback to handle BLE events. The events and 
                       associated actions taken by this application are inside the 
                       'StackEventHandler' routine. Note that Cortex M4 only handles 
                       the BLE host portion of the stack, while Cortex M0+ handles 
                       the BLE controller portion */
                    Cy_BLE_ProcessEvents();
                    break;
                
                /*~~~~~~~~~~~~~~ Command to start BLE ~~~~~~~~~~~~~~~~~~~~~~~~*/
                case START_BLE_ADV:
                    /* Enter into discoverable mode so that remote device can search it */
                    log_d("start ble adv\r\n");
                    StartAdvertisement();
                    Cy_BLE_ProcessEvents();
                    break;
                
                /*~~~~~~~~~~~~~~ Command to stop BLE ~~~~~~~~~~~~~~~~~~~~~~~~~*/
                case STOP_BLE_ADV:
                    log_d("stop ble adv\r\n");
                    bleApiResult = Cy_BLE_GAPP_StopAdvertisement();
                    Cy_BLE_ProcessEvents();
                    
                    if (bleApiResult != CY_BLE_SUCCESS ) 
                        log_d("Failure! : BLE - Advertisement API, " "Error code:[%x]", bleApiResult);
                    break;
             
                case UPDATA_ADVSCAN_DATA:
                    break;
                
                case CONN_PARAM_UPDATA:
                    if (Cy_BLE_GetConnectionState(ty_bal_get_conn_handle(0)) == CY_BLE_CONN_STATE_CONNECTED) {
                        log_d("req conn param updata\r\n");
                        ty_bal_conn_param_update(ty_bal_get_conn_handle(0).bdHandle, 20, 40, 0, 5000);
                    }
                    break;

                case START_BLE_SCAN:
                    //Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, cy_ble_scanIndex);
                    //Cy_BLE_ProcessEvents();
                    log_d("start ble scan\r\n");
                    break;
                
                case STOP_BLE_SCAN:
                    //Cy_BLE_GAPC_StopScan();
                    //Cy_BLE_ProcessEvents();
                    log_d("stop ble scan\r\n");
                    break;
                
                /*~~~~~~~~~~~~~~ Unknown command ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
                default:
                    log_d("Info     : BLE - Unknown BLE command");
                    break;
            }
        } else { 
            /* Task has timed out and received no commands during an interval of portMAXDELAY ticks */
            log_d("Warning! : BLE - Task Timed out ");
        }
    }
}

/*******************************************************************************
* Function Name: bool Task_Ble_Tickless_Idle_Readiness (void) 
********************************************************************************
* Summary:
*  This function returns the Tickless Idle readiness of Task_Ble
* 
* Parameters: 
*  None
*
* Return:
*  bool: returns the Tickless Idle readiness of Task_Ble
*******************************************************************************/
bool Task_Ble_Tickless_Idle_Readiness (void) 
{
    /* Return "true" if the BLE is on, "false" otherwise */
    return (Cy_BLE_GetState() == CY_BLE_STATE_ON) ? true : false;
}

/*******************************************************************************
* Function Name: static void StackEventHandler(uint32_t event, void *eventParam)
********************************************************************************
* Summary:
*  Call back event function to handle various events from the BLE stack. Note 
*  that Cortex M4 only handles the BLE host portion of the stack, while 
*  Cortex M0+ handles the BLE controller portion. 
*
* Parameters:
*  event        :	BLE event occurred
*  eventParam   :	Pointer to the value of event specific parameters
*
* Return:
*  None
*
*******************************************************************************/
static void StackEventHandler(uint32_t eventType, void *eventParam)
{   
    cy_en_ble_api_result_t apiResult;
    
    /* Local variable to store write request  parameter */
    static cy_stc_ble_gatts_write_cmd_req_param_t writeReqParameter;
    
    /* Local variable for storing connection handle */ 
    static cy_stc_ble_conn_handle_t connHandle; 
    
    static cy_stc_ble_bd_addrs_t ble_bd_addr;
    
    /* Take an action based on the current event */
    switch ((cy_en_ble_event_t)eventType)
    {
        /*~~~~~~~~~~~~~~~~~~~~~~ GENERAL  EVENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        
        /* This event is received when the BLE stack is Started */
        case CY_BLE_EVT_STACK_ON:
        {
            log_d("Info     : BLE - Stack on");
 
            /* Enter into discoverable mode so that remote device can search it */
            StartAdvertisement();
            break;
        }
        
        /* This event is used to inform the application that BLE Stack shutdown is completed */
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
        {
            log_d("Info     : BLE - Stack shutdown complete\r\n");
            log_d("Info     : Entering hibernate mode\r\n");
            
            /* Wait for UART transmission complete */
            WAIT_FOR_UART_TX_COMPLETE;    
            
            /* Enter hibernate mode */
            Cy_SysPm_Hibernate();
            break;
        }
        
        /* This event is received when there is a timeout */
        case CY_BLE_EVT_TIMEOUT:
        {    
            log_d("Info     : BLE - Event timeout");
            break;
        }
        
        /* This event indicates set device address command completed. */
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
        {
            //log_d("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            cy_stc_ble_events_param_generic_t *param = (cy_stc_ble_events_param_generic_t *) eventParam;
            //log_d("Set Local BDAddress [Status 0x%02X]\r\n", param->status);
            break;
        }
        
        /** This event indicates that the get device address command has completed.
         *  Event parameter returned with this event is of (cy_stc_ble_events_param_generic_t*) type.
         *  There are two members of the structure pointed to by the event parameter: status (uint8_t) and eventParams (void *).
         *  eventParams must be cast to (cy_stc_ble_bd_addrs_t*) type to access the public and private 
         *  Bluetooth Device Addresses.
         *  eventParams is valid only if status is success(0x00)
         */        
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
        {
            ble_bd_addr = *(cy_stc_ble_bd_addrs_t *)eventParam;
            break; 
        }
        
        /** This event indicates a new Bluetooth device address generated successfully as per an application requirement.
         *  The event parameter is cy_stc_ble_bd_addr_t 
         */
        case CY_BLE_EVT_GAP_DEVICE_ADDR_GEN_COMPLETE:
        {
            ble_bd_addr = *(cy_stc_ble_bd_addrs_t *)eventParam;
            break;
        }
        
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~ GATT EVENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        /** This event indicates that the 'GATT MTU Exchange Request' is received from GATT Client device. The event parameter
         *  is of 'cy_stc_ble_gatt_xchg_mtu_param_t' type and contains the Client RX MTU size.
         */
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        {
            cy_stc_ble_gatt_xchg_mtu_param_t *param = (cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam;
            log_d("Info     : BLE - GATTS exchange mtu rsp, mtu->[%d]", param->mtu);
            
            cy_stc_ble_gatt_xchg_mtu_param_t mtuParam;
            mtuParam.connHandle = param->connHandle;
            mtuParam.mtu = CY_BLE_GATT_MTU;
            (void)Cy_BLE_GATTS_ExchangeMtuRsp(&mtuParam);
            break;
        }
        
        /** This event indicates that the 'GATT MTU Exchange Response' is received from GATT Server device. The event parameter 
         *  is a pointer to a structure of type cy_stc_ble_gatt_xchg_mtu_param_t and contains the Server RX MTU size. 
         */
        case CY_BLE_EVT_GATTC_XCHNG_MTU_RSP:
        {
            cy_stc_ble_gatt_xchg_mtu_param_t *param = (cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam;
            log_d("Info     : BLE - GATTC exchange mtu rsp, mtu->[%d]", param->mtu);
            break;
        }
        
        /* This event is received when device is connected over GATT level */    
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            log_d("Info     : BLE - GATT connection established");
            
            /* Get connection handle of connected device */
            connHandle = *(cy_stc_ble_conn_handle_t*)eventParam;
            
            /* Updates the connection information */
            appConnHandle[connHandle.attId].attId    = connHandle.attId;
            appConnHandle[connHandle.attId].bdHandle = connHandle.bdHandle;
            break;
        }
        
        /* This event is received when device is disconnected */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
        {
            log_d("Info     : BLE - GATT disconnection occurred");
            
            /* Get connection handle of disconnected device */
            connHandle = *(cy_stc_ble_conn_handle_t*)eventParam;
            
            /* Update connection handle array */
            appConnHandle[connHandle.attId].attId    = connHandle.attId;
            appConnHandle[connHandle.attId].bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;                        
            break;
        }
        
        /* This event is received when Central device sends a write command
           on an Attribute */
        case CY_BLE_EVT_GATTS_WRITE_REQ:
        {
            writeReqParameter = *(cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam;

            log_d("Info     : BLE - GATT write request");    
            //log_d("attrHandle->[%x]\r\n", writeReqParameter.handleValPair.attrHandle);
            //elog_hexdump("handleVal", 32, writeReqParameter.handleValPair.value.val, writeReqParameter.handleValPair.value.len);
            
            /* Send the response to the write request received. */
            Cy_BLE_GATTS_WriteRsp(writeReqParameter.connHandle);

            /* Write request for Tuya Common service */
            if (writeReqParameter.handleValPair.attrHandle == TUYA_SPP_COMMON_CCCD_HANDLE) {
                /* Update GATT database with new value */    
                UpdateGattDB(writeReqParameter);
            }
            break;
        } 
        
        /* This event indicates that the 'Write Command' is received from GATT Client device. */
        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
        {
            writeReqParameter = *(cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam;    

            //log_d("Info     : BLE - GATT write cmd"); 
            //log_d("attrHandle->[%x]\r\n", writeReqParameter.handleValPair.attrHandle);
            //elog_hexdump("ble spp comm rcv data", 32, writeReqParameter.handleValPair.value.val, writeReqParameter.handleValPair.value.len);
            
            /* Update GATT database with new value */    
            UpdateGattDB(writeReqParameter);

            if (writeReqParameter.handleValPair.attrHandle == TUYA_SPP_COMMON_RX_HANDLE) {
                tuya_ble_gatt_receive_data((uint8_t*)(writeReqParameter.handleValPair.value.val), writeReqParameter.handleValPair.value.len);
            }
        }
        break;
        
        /* This event is received when Central device sends a read command on an Attribute */
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
        {
            log_d("Info     : BLE - GATT read request");
            break;
        }
        
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~ GAP EVENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        
        /* This event indicates peripheral device has started/stopped advertising */
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        {   
            log_d("Info     : BLE - Advertisement start/stop event");
            
            if (Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                /* Toggle Orange LED periodically to indicate that BLE is in a advertising state */
            }
            break;                
        }
       
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
        {
            log_d("Info     : BLE - Scan start/stop event");
        }
            break;
        
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        {
            cy_stc_ble_gapc_adv_report_param_t *param = (cy_stc_ble_gapc_adv_report_param_t *)eventParam;
            log_d("Info     : BLE - GAPC_SCAN, rssi->[%d] dlen->[%d]", param->rssi, param->dataLen);
        }
            break;
        
        /** This event indicates that the GAP Peripheral device has started/stopped advertising.
         *  This event is generated after making a call to the Cy_BLE_GAP_UpdateAdvScanData function.
         *  The event parameter contains the HCI Status error code, which is of type 'uint8_t'.
         *  If the data is '0x00', it indicates 'success'; anything else indicates 'failure'. 
         */
        case CY_BLE_EVT_GAPP_UPDATE_ADV_SCAN_DATA_COMPLETE:
        {
            uint8_t error_code = *(uint8_t *)eventParam;
            log_d("Info     : BLE - GAP UpdateAdvScanData, ret->[%d]", error_code);
            break;
        }
        
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
        case CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
        {  
            cy_stc_ble_gap_enhance_conn_complete_param_t conn_param = *(cy_stc_ble_gap_enhance_conn_complete_param_t *)eventParam;
                    
            log_d("Info     : BLE - GAP device connected");
            log_d("Info     : Connected device->[%d]; [connIntv->[%d]*1.25ms, connLatency->[%d], supervisionTo->[%d]*10ms] ", \
                    Cy_BLE_GetNumOfActiveConn(), conn_param.connIntv, conn_param.connLatency, conn_param.supervisionTo);
            
            /* Conn param updata request */ 
            ble_conn_param_updata_timer_reset();
            
            tuya_ble_connected_handler();
            
            /* Start advertisement for next device. */
            StartAdvertisement();
            break;
        }
        
        /* This event is generated when disconnected from remote device or 
           failed to establish connection */
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        {    
            log_d("Info     : BLE - GAP device disconnected");
            log_d("Info     : Connected device->[%d]", Cy_BLE_GetNumOfActiveConn());
             
            cy_stc_ble_gap_disconnect_param_t *param = (cy_stc_ble_gap_disconnect_param_t *) eventParam;
            log_d("disconnect reason ->[%x]\r\n", param->reason);
            
            extern void ota_disconn_handler(void);
            ota_disconn_handler();
            tuya_ble_disconnected_handler();
            
            /* Start advertisement for next device. */
            StartAdvertisement();
            break;            
        }
        
        /** This event indicates the connection parameter update response is received
         *  from the master. The event parameter is a pointer to a structure of type
         *  cy_stc_ble_l2cap_conn_update_rsp_param_t. 
         */
        case CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP: 
        {
            cy_stc_ble_l2cap_conn_update_rsp_param_t *param = ((cy_stc_ble_l2cap_conn_update_rsp_param_t *)eventParam);
            log_d("conn param updata rsp, bdHandle->[%d], result->[%d]\r\n", param->bdHandle, param->result);
            break;
        }
        
        /** This event is generated at the GAP Central and the Peripheral end after a connection parameter update
         *  is requested from the host to the controller.
         *  The event parameter is a pointer to a structure of type cy_stc_ble_gap_conn_param_updated_in_controller_t. 
         */
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
        {
            cy_stc_ble_gap_conn_param_updated_in_controller_t *param = ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam);
            log_d("connection parameter update complete\r\n");
            log_d("connIntv->[%d]*1.25ms, connLatency->[%d], supervisionTo->[%d]*10ms", param->connIntv, param->connLatency, param->supervisionTO);
            break;
        }
        
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~ OTHER EVENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/ 
        
        /* See the data-type cy_en_ble_event_t to understand the event occurred */
        default:
        {
            /* Other BLE events */
            break;
        }
    }
} 

/*******************************************************************************
* Function Name: static void BleControllerInterruptEventHandler(void)
********************************************************************************
* Summary:
*  Call back event function to handle interrupts from BLE Controller
*  (Cortex M0+)
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void BleControllerInterruptEventHandler(void) {
    /* Send command to process BLE events  */
    ble_msg_t ble_msg = {.command = PROCESS_BLE_EVENTS};
    ty_hal_os_msg_queue_send_from_isr(ble_queue_handle, &ble_msg);
}

/*******************************************************************************
* Function Name: static void StartAdvertisement(void)
********************************************************************************
* Summary:
*  This function starts the advertisement if not already advertising and connected
*  device count is less than the maximum allowed devices.
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
static void StartAdvertisement(void) {
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    
    /* Get the number of active connection */
    uint8_t numActiveConn = Cy_BLE_GetNumOfActiveConn();

    /* Start Advertisement and enter discoverable mode.
       Make sure that BLE is neither connected nor advertising already */
    if ((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED) && \
        (numActiveConn < CY_BLE_CONN_COUNT)) {
        bleApiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX); 
        if (bleApiResult != CY_BLE_SUCCESS ) {
            log_d("Failure! : BLE - Advertisement API, " "Error code:[%d]", bleApiResult);
        }
    } 
}

/*******************************************************************************
* Function Name: UpdateCccdStatusInGattDb
********************************************************************************
* Summary:
*  This function updates the notification status (lower byte of CCCD array) of
*  a characteristic in GATT DB with the provided parameters
*
* Parameters:
*  cccdHandle   :	CCCD handle of the service
*  appConnHandle:   Connection handle
*  value        :   Notification status. Valid values are CCCD_NOTIFY_DISABLED and
*                   CCCD_NOTIFY_ENABLED
*
* Return:
*  void
*
*******************************************************************************/
void UpdateCccdStatusInGattDb(cy_ble_gatt_db_attr_handle_t cccdHandle, cy_stc_ble_conn_handle_t appConnHandle, uint8_t value) {
    /* Local variable to store the current CCCD value */
    uint8_t cccdValue[CY_BLE_CCCD_LEN];
    
    /* Load the notification status to the CCCD array */
    cccdValue[0] = value;
    cccdValue[0] = CY_BLE_CCCD_DEFAULT;
        
    /* Local variable that stores notification data parameters */
    cy_stc_ble_gatt_handle_value_pair_t  cccdValuePair = 
    {
        .attrHandle = cccdHandle,
        .value.len = CY_BLE_CCCD_LEN,
        .value.val = cccdValue
    };
    
    /* Local variable that stores attribute value */
    cy_stc_ble_gatts_db_attr_val_info_t  cccdAttributeHandle=
    {
        .connHandle = appConnHandle,
        .handleValuePair = cccdValuePair,
        .offset = CY_BLE_CCCD_DEFAULT,
    };
    
    /* Extract flag value from the connection handle - TO BE FIXED*/
    if(appConnHandle.bdHandle == 0u)
    {
        cccdAttributeHandle.flags = CY_BLE_GATT_DB_LOCALLY_INITIATED;
    }
    else
    {
        cccdAttributeHandle.flags = CY_BLE_GATT_DB_PEER_INITIATED;
    }
    
    /* Update the CCCD attribute value per the input parameters */
    Cy_BLE_GATTS_WriteAttributeValueCCCD(&cccdAttributeHandle);
}

/*******************************************************************************
* Function Name: static void UpdateGattDB(
*                       cy_stc_ble_gatts_write_cmd_req_param_t writeReqParam)
********************************************************************************
*
* Summary:
*  This function update the value field of the specified attribute  in the GATT 
*  database of a GATT Server by a peer device.
*
* Parameters:  
*  cy_stc_ble_gatts_write_cmd_req_param_t writeReqParam: Write request paramete
*
* Return: 
*  None
*
*******************************************************************************/
static void UpdateGattDB(cy_stc_ble_gatts_write_cmd_req_param_t writeReqParam)
{
    /* Local variable that stores custom service data parameters */
    cy_stc_ble_gatt_handle_value_pair_t  handleValue = 
    {
        .attrHandle = writeReqParam.handleValPair.attrHandle,
        .value      = writeReqParam.handleValPair.value,
    };
    Cy_BLE_GATTS_WriteAttributeValuePeer(&writeReqParam.connHandle, &handleValue);
}

/* [] END OF FILE */

