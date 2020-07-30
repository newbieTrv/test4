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
#include "custom_app_product_test.h"
#include "tuya_ble_main.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_internal_config.h"
#include "board_adapter.h"

static bool is_ftm_pcba_mode(void);
static void ftm_pcba_mode_entry(void);

tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void)
{
    // 
    ftm_pcba_mode_entry();
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void)
{
    //
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi)
{
    //
    *rssi = -30;
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_gpio_test(void)
{
    //Add gpio test code here
    return TUYA_BLE_SUCCESS;
}


void tuya_ble_custom_app_production_test_process(uint8_t channel, uint8_t *p_in_data, uint16_t in_len) {
    uint16_t cmd = 0;
    uint8_t *data_buffer = NULL;
    uint16_t data_len = ((p_in_data[4]<<8) + p_in_data[5]);

    if ((p_in_data[6] != 3) || (data_len < 3))
        return;
  
    cmd = (p_in_data[7]<<8) + p_in_data[8];
    data_len -= 3;
    if (data_len > 0)
    {
        data_buffer = p_in_data+9;
    }
    
    // TODO .. YOUR JOBS
    
}


static entry_ftm_pcba_stu_t entry_ftm_pcba_stu;

/* Timer handles */
#define FTM_PCBA_TIMER_NUMS             ( 2 )
#define FTM_PCBA_TIMER_ID_ENTRY_TIMEOUT ( 0 )
#define FTM_PCBA_TIMER_ID_EXIT_REBOOT   ( 1 )

static void FtmPcbaTimerCallback_entry_timeout(void *xTimer);
static void FtmPcbaTimerCallback_exit_reboot(void *xTimer);

void *xFtmPcbaTimerHandle[FTM_PCBA_TIMER_NUMS] = {NULL};
static xTimeParas xFtmPcbaTimer[FTM_PCBA_TIMER_NUMS] = {
	{(1000),     TimerOnce,    	   0,     	"0", 		FtmPcbaTimerCallback_entry_timeout}, 
    {(2000),     TimerOnce,    	   1,     	"1", 		FtmPcbaTimerCallback_exit_reboot}, 
};

static void create_ftm_pcba_related_timer(void) {
    /* Create an RTOS timer */
    for (int id=0; id<FTM_PCBA_TIMER_NUMS; id++) {
        tuya_ble_timer_create(&xFtmPcbaTimerHandle[id], \
					xFtmPcbaTimer[id].period, \
					xFtmPcbaTimer[id].type, \
					xFtmPcbaTimer[id].cb); 
						
        if (xFtmPcbaTimerHandle[id] == NULL) 
            TUYA_APP_LOG_ERROR("create ftm pcba timer failed, timer id->[%d]\n", id);
    }  
}   

static void ftm_pcba_entry_timeout_timer_start(uint32_t ticks) {
    int id = FTM_PCBA_TIMER_ID_ENTRY_TIMEOUT;
    tuya_ble_timer_restart(xFtmPcbaTimerHandle[id], ticks);
}

static void ftm_pcba_entry_timeout_timer_stop(void) {
    int id = FTM_PCBA_TIMER_ID_ENTRY_TIMEOUT;
    tuya_ble_timer_stop(xFtmPcbaTimerHandle[id]);
}

static void ftm_pcba_exit_reboot_timer_start(uint32_t ticks) {
    int id = FTM_PCBA_TIMER_ID_EXIT_REBOOT;
    tuya_ble_timer_restart(xFtmPcbaTimerHandle[id], ticks);
}

static void FtmPcbaTimerCallback_entry_timeout(void *xTimer) {
    (void)(xTimer);
	entry_ftm_pcba_stu.timeout = true;
            
	if (!entry_ftm_pcba_stu.entry) {
		ty_pdl_ftm_uart_ops.ftm_uart_close();
        TUYA_APP_LOG_INFO("entry ftm timeout, close ftm uart\r\n");
    }
}
        
static void FtmPcbaTimerCallback_exit_reboot(void *xTimer) {
    (void)(xTimer);
	ty_hal_system_reboot(); 
}

static bool is_ftm_pcba_mode(void) {
    return entry_ftm_pcba_stu.entry;
}   

static void ftm_pcba_mode_entry(void) {
    if (entry_ftm_pcba_stu.entry)
        return;
    
    ftm_pcba_entry_timeout_timer_stop();
    entry_ftm_pcba_stu.entry = true;
    entry_ftm_pcba_stu.timeout = false;
    entry_ftm_pcba_stu.last_cmd = 0xFFFF;
    entry_ftm_pcba_stu.pdl_test_stu.stu = 0;
    TUYA_APP_LOG_INFO("into pcba test mode\r\n");
}

static void ftm_pcba_mode_exit(void) {
    entry_ftm_pcba_stu.entry = false;
    entry_ftm_pcba_stu.timeout = false;
    entry_ftm_pcba_stu.last_cmd = 0xFFFF;
    entry_ftm_pcba_stu.pdl_test_stu.stu = 0;

    /* exit ftm, do system reset */
    TUYA_APP_LOG_INFO("exit pcba test mode\r\n");
    ftm_pcba_exit_reboot_timer_start(2000);
}

static void ftm_pcba_response(uint8_t channel, uint16_t sub_cmd, uint8_t *data, uint16_t size) {
    uint8_t *rsp;
    uint16_t rsp_len;
    uint16_t data_len = size + 3;

    rsp = (uint8_t *)tuya_ble_malloc(16+size);
    if (rsp == NULL)
    return;
    
    rsp[0] = TY_FTM_PROTOCOL_FIRST_HEAD;
    rsp[1] = TY_FTM_PROTOCOL_SECOND_HEAD;
    rsp[2] = TY_FTM_PROTOCOL_VERSION;
    rsp[3] = TY_FTM_PCBA_CMD_SET;
    rsp[4] = (uint8_t)(data_len >> 8);
    rsp[5] = (uint8_t)(data_len);
    rsp[6] = PROTOCOL_TYPE_BLE;
    rsp[7] = (uint8_t)(sub_cmd >> 8);
    rsp[8] = (uint8_t)(sub_cmd);
    memcpy(&rsp[9], data, size);
    rsp_len = data_len + 6;

    rsp[rsp_len] = tuya_ble_check_sum(rsp, rsp_len);
    rsp_len += 1;
	tuya_ble_production_test_asynchronous_response(channel, rsp, rsp_len);
    
    tuya_ble_free(rsp);
}
    
void tuya_ble_ftm_init(void) {
    entry_ftm_pcba_stu.entry   = false;
    entry_ftm_pcba_stu.timeout = false;
    entry_ftm_pcba_stu.last_cmd = 0xFFFF;
    entry_ftm_pcba_stu.pdl_test_stu.stu = 0;
    
    create_ftm_pcba_related_timer();

    /* into factory product test by uart, setting timeout period */
    ty_pdl_ftm_uart_ops.ftm_uart_open(FTM_UART_BAUD_RATE);   
    ftm_pcba_entry_timeout_timer_start(1000);
}

/* [] END OF FILE */
