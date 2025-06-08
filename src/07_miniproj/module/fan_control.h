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
 * Abstract: Fan Control Header
 *
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */
#ifndef _FAN_CONTROL_H__
#define _FAN_CONTROL_H__
#include "config.h"

/**
 * @brief Initializes the fan control module.
 * Starts the fan control thread
 */
int fan_control_init(void);

/**
 * @brief Deinitializes the fan control module.
 * Stops the fan control thread and cleans up resources
 */
void fan_control_deinit(void);

/**
 * @brief Sets the fan mode.
 * @param mode The desired fan mode (FAN_MODE_AUTO or FAN_MODE_MANUAL).
 * This function will adjust the fan speed or LED blink frequency based on the mode.
 * In manual mode, the fan speed will be set to a fixed value.
 * In auto mode, the fan speed will be adjusted based on the CPU temperature.
 * @note This function is thread-safe and can be called from any context.
 */
void fan_control_set_mode(const enum fan_mode mode);

/**
 * @brief Sets the blink frequency of the status LED.
 * @param freq The desired blink frequency in Hz.
 * If the mode is set to manual, this will control the blink frequency of the status LED.
 * If the mode is set to auto, this will not have any effect, until the mode is changed to manual.
 * @note This function is thread-safe and can be called from any context.
 */
void fan_control_set_blink_freq(unsigned int freq);

/**
 * @brief Gets the current blink frequency of the status LED.
 * @return The current blink frequency in Hz.
 */
int fan_control_get_blink_freq(void);

/**
 * @brief Gets the current fan mode.
 * @return The current fan mode (FAN_MODE_AUTO or FAN_MODE_MANUAL).
 */
const enum fan_mode fan_control_get_mode(void);

#endif // _FAN_CONTROL_H__