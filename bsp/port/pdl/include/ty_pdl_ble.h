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
#ifndef __TY_PDL_BLE_H__
#define __TY_PDL_BLE_H__

#include "board_adapter.h"
    
/* Adv interval res val */    
#define BLE_APP_ADVINTERVAL_RES    ( 1.6 )
    
enum {
    UNIT_0_625_MS = 625,        /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250,       /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000       /**< Number of microseconds in 10 milliseconds. */
};

/**@brief Macro for converting milliseconds to ticks.
 *
 * @param[in] TIME          Number of milliseconds to convert.
 * @param[in] RESOLUTION    Unit to be converted to in [us/ticks].
 */
#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))
    

/* List of BLE commands */
typedef enum  {
    PROCESS_BLE_EVENTS,             /* Process pending Stack event*/ 
    START_BLE_ADV,
    STOP_BLE_ADV,      
    UPDATA_ADVSCAN_DATA, 
    CONN_PARAM_UPDATA,
    
    START_BLE_SCAN,
    STOP_BLE_SCAN, 
} ble_cmd_list_t;

/* Data-type of BLE commands and data */
typedef struct {   
    ble_cmd_list_t command;
    void* data;
} ble_msg_t;


#ifdef __cplusplus
 extern "C" {
#endif 

extern cy_stc_ble_conn_handle_t ty_bal_get_conn_handle(uint8_t ble_conn_index);
extern cy_en_ble_api_result_t ty_bal_get_mac_address(uint8_t *mac);
extern cy_en_ble_api_result_t ty_bal_set_mac_address(cy_stc_ble_gap_bd_addr_t *p_addr);
extern void ty_bal_advertising_start(void);
extern void ty_bal_advertising_stop(void);
extern void ty_bal_advertising_init(void);
extern void ty_bal_set_adv_interval(uint32_t adv_inv /* unit:ms */);
extern bool ty_bal_adv_update_adv_data(uint8_t const* p_adv_data, uint8_t adv_len);
extern bool ty_bal_adv_update_scan_rsp_data(uint8_t const *p_sr_data, uint8_t sr_len);
extern bool ty_bal_conn_param_update(uint8_t bdHandle, uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout);
extern bool ty_bal_mtu_exchange_req(uint16_t gatt_mtu);
extern cy_en_ble_api_result_t ty_bal_gatts_send_notfication(const uint8_t *data, uint16_t length);
extern cy_en_ble_api_result_t ty_bal_gap_disconnect(void);

extern void ble_pdl_task_init(void);
/* Function that returns the Tickless Idle readiness of Task_Ble */
extern bool Task_Ble_Tickless_Idle_Readiness (void);

#ifdef __cplusplus
}
#endif 

#endif // __TY_PDL_BLE_H__

/* [] END OF FILE */
