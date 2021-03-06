/**
 * \file tuya_ble_port_psco6.h
 *
 * \brief 
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk 
 */


#ifndef __TUYA_BLE_PORT_PSOC6_H__
#define __TUYA_BLE_PORT_PSOC6_H__


#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_config.h"
#include "board_adapter.h"
    
    
tuya_ble_status_t cy_ble_api_err_code_convert(uint32_t err_code);

#if (TUYA_BLE_LOG_ENABLE || TUYA_APP_LOG_ENABLE)
    
#define TUYA_BLE_PRINTF(...)            log_d(__VA_ARGS__)
#define TUYA_BLE_HEXDUMP(...)           elog_hexdump("", 8, __VA_ARGS__)
    
#else
    
#define TUYA_BLE_PRINTF(...)           
#define TUYA_BLE_HEXDUMP(...)  

#endif

#endif // __TUYA_BLE_PORT_PSOC6_H__






