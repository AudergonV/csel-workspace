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
 * Abstract: Fan Control Implementation
 * - Create a thread to read the CPU temperature, and adjust the led blink frequency accordingly
 * - Since there is no fan, the thread will only adjust the blink frequency of the status LED
 *
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */

#include "fan_control.h"
#include "status_led.h"
#include "cpu_temperature.h"

#include <linux/kthread.h>
#include <linux/delay.h>

struct task_struct *fan_thread = NULL;
int _blink_frequency_manual = 1;
int _actual_blink_frequency = 1;
enum fan_mode _mode = FAN_MODE_AUTO;

static int fan_control_thread(void *data) {
    int temp;
    while (!kthread_should_stop()) {
        // Read CPU temperature
        if (cpu_temperature_get(&temp) == 0) {
            pr_info("Current CPU temperature: %d°C\n", temp);
            // Adjust blink frequency based on temperature
            if (_mode == FAN_MODE_AUTO) {
                if (temp < 35) {
                    _actual_blink_frequency = 2; // Below 35°C
                } else if (temp < 40) {
                    _actual_blink_frequency = 5; // Between 35°C and 40°C
                } else if (temp < 45) {
                    _actual_blink_frequency = 10; // Between 40°C and 45°C
                } else {
                    _actual_blink_frequency = 20; // Above 45°C
                }
            } else if (_mode == FAN_MODE_MANUAL) {
                _actual_blink_frequency = _blink_frequency_manual;
            }
        } else {
            pr_err("Failed to read CPU temperature\n");
        }
        status_led_set_blink_freq(_actual_blink_frequency);
        msleep(1000); // Sleep for 1 second
    }
    return 0;
}

int fan_control_init(void) {
    status_led_init();
    fan_thread = kthread_run(fan_control_thread, NULL, "fan_control_thread");
    if (IS_ERR(fan_thread)) {
        pr_err("Failed to create fan control thread\n");
        fan_thread = NULL;
        return PTR_ERR(fan_thread);
    }
    return 0;
}

void fan_control_deinit(void) {
    status_led_deinit();
    if (fan_thread) {
        kthread_stop(fan_thread);
        fan_thread = NULL;
    }
}

void fan_control_set_mode(const enum fan_mode mode) {
    if (mode != FAN_MODE_AUTO && mode != FAN_MODE_MANUAL) {
        pr_err("Invalid fan mode: %d\n", mode);
        return;
    }
    _mode = mode;
    pr_info("Fan mode set to: %s\n", mode == FAN_MODE_AUTO ? "AUTO" : "MANUAL");
}

void fan_control_set_blink_freq(int freq) {
    _blink_frequency_manual = freq;
}

int fan_control_get_blink_freq(void) {
    return _actual_blink_frequency;
}

const enum fan_mode fan_control_get_mode(void) {
    return _mode;
}