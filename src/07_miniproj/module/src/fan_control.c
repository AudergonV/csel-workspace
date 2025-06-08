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
#include "config.h"

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/atomic.h>

struct task_struct *fan_thread = NULL;
static atomic_t blink_frequency_manual = ATOMIC_INIT(CSEL_DEFAULT_BLINK_FREQ_MANUAL_HZ);
static atomic_t actual_blink_frequency = ATOMIC_INIT(CSEL_DEFAULT_BLINK_FREQ_MANUAL_HZ);
static atomic_t mode_atomic = ATOMIC_INIT(CSEL_DEFAULT_MODE);

static const unsigned int temperature_thresholds[] = CSEL_TEMPERATURE_THRESHOLDS_C;
static const unsigned int blink_frequencies[] = CSEL_BLINK_FREQUENCIES_HZ;
static const unsigned int num_thresholds = sizeof(temperature_thresholds) / sizeof(temperature_thresholds[0]);

static int calculate_blink_frequency(int temperature) {
    int i;
    for (i = 0; i < num_thresholds; i++) {
        if (temperature < temperature_thresholds[i]) {
            return blink_frequencies[i];
        }
    }
    return blink_frequencies[num_thresholds - 1];
}

static int fan_control_thread(void *data) {
    int temp;
    while (!kthread_should_stop()) {
        if (cpu_temperature_get(&temp) == 0) {
            // Adjust blink frequency based on temperature
            int mode = atomic_read(&mode_atomic);
            if (mode == FAN_MODE_AUTO) {
                int freq = calculate_blink_frequency(temp);
                atomic_set(&actual_blink_frequency, freq);
            } else if (mode == FAN_MODE_MANUAL) {
                int manual = atomic_read(&blink_frequency_manual);
                atomic_set(&actual_blink_frequency, manual);
            }
        } else {
            pr_err("failed to read CPU temperature\n");
        }
        status_led_set_blink_freq(atomic_read(&actual_blink_frequency));
        msleep(CSEL_TEMPERATURE_POLLING_PERIOD_MS);
    }
    return 0;
}

int fan_control_init(void) {
    int status = cpu_temperature_init();
    if (status < 0) {
        pr_err("failed to initialize CPU temperature sensor: %d\n", status);
        return status;
    }
    status = status_led_init();
    if (status < 0) {
        pr_err("failed to initialize status LED: %d\n", status);
        return status;
    }
    fan_thread = kthread_run(fan_control_thread, NULL, "fan_control_thread");
    if (IS_ERR(fan_thread)) {
        pr_err("failed to create fan control thread\n");
        fan_thread = NULL;
        return PTR_ERR(fan_thread);
    }
    return 0;
}

void fan_control_deinit(void) {
    if (fan_thread) {
        kthread_stop(fan_thread);
        fan_thread = NULL;
    }
    status_led_deinit();
}

void fan_control_set_mode(const enum fan_mode mode) {
    if (mode != FAN_MODE_AUTO && mode != FAN_MODE_MANUAL) {
        pr_err("invalid fan mode: %d\n", mode);
        return;
    }
    atomic_set(&mode_atomic, mode);

    if (mode == FAN_MODE_AUTO) {
        int temp = 0;
        if (cpu_temperature_get(&temp) == 0) {
            int freq = calculate_blink_frequency(temp);
            atomic_set(&actual_blink_frequency, freq);
            status_led_set_blink_freq(freq);
        }
    }
}

void fan_control_set_blink_freq(unsigned int freq) {
    atomic_set(&blink_frequency_manual, freq);
}

int fan_control_get_blink_freq(void) {
    return atomic_read(&actual_blink_frequency);
}

const enum fan_mode fan_control_get_mode(void) {
    return (enum fan_mode)atomic_read(&mode_atomic);
}