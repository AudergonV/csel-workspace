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
 * Abstract: CPU Temperature
 *
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */
#include <linux/thermal.h>

#include "cpu_temperature.h"

#define MILLI_CELIUS_TO_CELSIUS (1000)

struct thermal_zone_device* tz;

int cpu_temperature_init(void)
{
    tz = thermal_zone_get_zone_by_name(CSEL_THERMAL_ZONE_CPU);
    if (IS_ERR(tz)) {
        return PTR_ERR(tz);
    }
    pr_info("CPU temperature sensor initialized successfully\n");
    return 0;
}

void cpu_temperature_deinit(void)
{
    if (tz) {
        thermal_zone_device_unregister(tz);
        pr_info("CPU temperature sensor deinitialized successfully\n");
    } else {
        pr_warn("CPU temperature sensor was not initialized\n");
    }
}

int cpu_temperature_get(int *temp)
{
    int ret = thermal_zone_get_temp(tz, temp);
    if (ret < 0) {
        pr_err("Failed to get CPU temperature: %d\n", ret);
        return ret;
    }
    *temp /= MILLI_CELIUS_TO_CELSIUS;
    return 0;
}