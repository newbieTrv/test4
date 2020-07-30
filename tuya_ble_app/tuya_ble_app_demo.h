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
#ifndef __TUYA_BLE_APP_DEMO_H__
#define __TUYA_BLE_APP_DEMO_H__

// 产品PID 
#define TUYA_DEVICE_PID         ( "xxxxxxxx" )   
    
// 固件标识名
#define TUYA_DEVICE_FIR_NAME    ( "tuya_ble_sdk_app_demo_psoc63" )

// 固件版本
#define TUYA_DEVICE_FVER_NUM    ( 0x0000100 )    
#define TUYA_DEVICE_FVER_STR    ( "1.0" )       

// 硬件版本
#define TUYA_DEVICE_HVER_NUM    ( 0x00000100 )   
#define TUYA_DEVICE_HVER_STR    ( "1.0" )        
    
    
#ifdef __cplusplus
extern "C" {
#endif
 
extern void tuya_ble_app_task_init(void);

#ifdef __cplusplus
}
#endif

#endif // __TUYA_BLE_APP_DEMO_H__

/* [] END OF FILE */
