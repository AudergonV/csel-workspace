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
 * Abstract: CPU Temperature Header
 *
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */

#ifndef _CPU_TEMPERATURE_H__
#define _CPU_TEMPERATURE_H__
#include "config.h"

/**
 * @brief Initializes the CPU temperature sensor.
 * @return 0 on success, negative error code on failure.
 */
int cpu_temperature_init(void);

/**
 * @brief Gets the current CPU temperature.
 * @param temp Pointer to an integer where the temperature will be stored in Celsius.
 * @return 0 on success, negative error code on failure.
 */
int cpu_temperature_get(int *temp);

#endif // _CPU_TEMPERATURE_H__