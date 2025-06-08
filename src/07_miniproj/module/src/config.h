/**
 * Copyright 2025 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project: HEIA-FR / HES-SO MSE - MA-CSEL1 Mini Project
 *
 * Abstract: Kernel module - Configuration header
 * 
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */

#ifndef _CONFIG_H__
#define _CONFIG_H__

#undef pr_fmt
#define pr_fmt(fmt) "csel: " fmt  ///< Prefix for all printk messages in this module

enum fan_mode {
    FAN_MODE_AUTO,
    FAN_MODE_MANUAL
};

#define CSEL_MODULE_NAME "csel"                     ///< Name of the module, and platform device
#define CSEL_THERMAL_ZONE_CPU "cpu-thermal"         ///< Thermal zone name for CPU temperature
#define CSEL_DEFAULT_BLINK_FREQ_MANUAL_HZ 5U        ///< Default blink frequency in manual mode
#define CSEL_DEFAULT_MODE FAN_MODE_AUTO             ///< Default fan mode at module load
#define CSEL_MIN_BLINK_FREQ_MANUAL_HZ 1U            ///< Minimum blink frequency in manual mode
#define CSEL_MAX_BLINK_FREQ_MANUAL_HZ 20U           ///< Maximum blink frequency in manual mode
#define CSEL_STATUS_LED_GPIO 10U                    ///< GPIO pin for the status LED
#define CSEL_SYSFS_MODE_AUTO "auto"                 ///< String representing auto mode in sysfs     
#define CSEL_SYSFS_MODE_MANUAL "manual"             ///< String representing manual mode in sysfs
#define CSEL_TEMPERATURE_POLLING_PERIOD_MS 100U      ///< Temperature polling period in milliseconds

// Auto mode blink frequency settings
#define CSEL_TEMPERATURE_THRESHOLDS_C {35, 40, 45,  __INT_MAX__}
#define CSEL_BLINK_FREQUENCIES_HZ     {2U, 5U, 10U, 20U} 
#endif // _CONFIG_H__