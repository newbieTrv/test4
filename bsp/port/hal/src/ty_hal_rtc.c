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
#include "ty_hal_rtc.h"
#include "board_adapter.h"
#include "tuya_ble_unix_time.h"

/* Enabled SuperCap charging macro */
#define RTC_SUPERCAP_CHARGING   ( 0 )

/* RTC backup magicnum */
#define RTC_INITED_MAGICNUM     ( 0x11112288 )

/* Wait time between two RTC APIs */ 
#define RTC_API_WAIT_TIME       (3*1000/CYDEV_CLK_BAKCLK__KHZ)

void ty_hal_rtc_init(void) {
    /* Initialize RTC */
    cy_stc_rtc_config_t const rtc_init_config = {
        .sec       = 0,
        .min       = 0,
        .hour      = 4,
        .hrFormat  = CY_RTC_24_HOURS,
        .dayOfWeek = 3,
        .date      = 1,
        .month     = 1,
        .year      = 20,
    };
    
    /* Enabled SuperCap charging */
#if (RTC_SUPERCAP_CHARGING)
    Cy_SysPm_SetBackupSupply(CY_SYSPM_VDDBACKUP_DEFAULT);
    Cy_SysPm_BackupSuperCapCharge(CY_SYSPM_SC_CHARGE_ENABLE);
#endif // RTC_ENABLE_SUPERCAP_CHARGING
    
    uint32 rtc_backup_magicnum = CY_GET_REG32(CYREG_BACKUP_BREG0);
    log_d("rtc_backup_magicnum->[%x]\r\n", rtc_backup_magicnum);
    if (rtc_backup_magicnum != RTC_INITED_MAGICNUM) {
        if (Cy_RTC_Init(&rtc_init_config) != CY_RTC_SUCCESS) {
            log_d("RTC initialization failed \r\n");
            CY_ASSERT(0); 
        }
        
        log_d("RTC initialization success\r\n");
        CY_SET_REG32(CYREG_BACKUP_BREG0, RTC_INITED_MAGICNUM);
    }
}

void ty_hal_rtc_get_datatime(cy_stc_rtc_config_t *dateTime) {
    Cy_RTC_GetDateAndTime(dateTime);
}

void ty_hal_rtc_set_datatime(uint32_t sec, uint32_t min, uint32_t hour, uint32_t date, uint32_t month, uint32_t year) {
    cy_en_rtc_status_t rtcApiResult;

    /* Set rtc date and time */
    rtcApiResult = Cy_RTC_SetDateAndTimeDirect(sec, min, hour, date, month, year);
    if (rtcApiResult != CY_RTC_SUCCESS) {
        log_d("Failure! : RTC initialization. Error Code->[%x]", rtcApiResult);
        CY_ASSERT(0u);
    }
    
    Cy_SysLib_Delay(RTC_API_WAIT_TIME);
}

uint32_t ty_hal_get_rtc_time(void) {
    tuya_ble_time_struct_data_t datatime;
    cy_stc_rtc_config_t rtc_datatime;
    uint32_t timestamp;

    ty_hal_rtc_get_datatime(&rtc_datatime);
    
    datatime.nYear  = (rtc_datatime.year+2000);
    datatime.nMonth = rtc_datatime.month;
    datatime.nDay   = rtc_datatime.date;
    datatime.nHour  = rtc_datatime.hour;
    datatime.nMin   = rtc_datatime.min;
    datatime.nSec   = rtc_datatime.sec;
    datatime.DayIndex = rtc_datatime.dayOfWeek;
    timestamp = tuya_ble_mytime_2_utc_sec(&datatime, 0);
    
    //log_d("timestamp->[%d]\r\n", timestamp);
    //log_d("get rtc %d-%d_%d %d:%d:%d\r\n", rtc_datatime.year, rtc_datatime.month, rtc_datatime.date, rtc_datatime.hour, rtc_datatime.min, rtc_datatime.sec);
    return timestamp;
}

uint32_t ty_hal_set_rtc_time(uint32_t timestamp, int32_t timezone) {
    (void)(timezone);
    tuya_ble_time_struct_data_t datatime;
    
    tuya_ble_utc_sec_2_mytime(timestamp, &datatime, 0);
    log_d("set rtc time: %d-%d-%d %d:%d:%d\r\n", datatime.nYear, datatime.nMonth, datatime.nDay, datatime.nHour, datatime.nMin, datatime.nSec);
    
    ty_hal_rtc_set_datatime(datatime.nSec, datatime.nMin, datatime.nHour, datatime.nDay, datatime.nMonth, (datatime.nYear-2000));            
    return 0;
}

/* [] END OF FILE */
