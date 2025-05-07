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
 * Project: HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Autĥor:  Vincent Audergon
 * Date:    15.04.2025
 */

#include "button.h"

#include <time.h>

#include "convert.h"

int button_init(button_t* b,
                int gpio,
                unsigned int press_time_until_repeat_ms,
                unsigned int repeat_period_ms)
{
    if (gpio_init(&b->gpio_ro, gpio, GPIO_DIR_IN) == -1) {
        return -1;
    }
    b->press_time_until_repeat_ms = press_time_until_repeat_ms;
    b->repeat_period_ms           = repeat_period_ms;
    b->_last_change               = time(NULL);
    b->_last_repeat               = time(NULL);
    b->_state                     = false;
    b->_repeat_triggered          = false;
    b->_on_click                  = NULL;
    b->_on_repeat                 = NULL;
    return 0;
}

void button_attach_on_click(button_t* b, void (*func)(void*), void* param)
{
    b->_on_click       = func;
    b->_on_click_param = param;
}
void button_attach_on_repeat(button_t* b, void (*func)(void*), void* param)
{
    b->_on_repeat       = func;
    b->_on_repeat_param = param;
}

static uint64_t current_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec) * SEC_TO_MS_FACTOR +
           ts.tv_nsec / MS_TO_NS_FACTOR;
}

void button_update(button_t* b)
{
    bool state   = gpio_read(&b->gpio_ro);
    uint64_t now = current_time_ms();
    if (state != b->_state) {
        b->_last_change = now;
        if (state == true) {  // rising edge
            if (b->_on_click != NULL) {
                b->_on_click(b->_on_click_param);
            }
        }
        b->_repeat_triggered = false;  // reset repeat
    }
    if (b->_state == true && !b->_repeat_triggered) {  // still pressed
        if (now - b->_last_change > b->press_time_until_repeat_ms) {
            b->_repeat_triggered = true;
            b->_last_repeat      = now;
            if (b->_on_repeat != NULL) {
                b->_on_repeat(b->_on_repeat_param);
            }
        }
    }
    if (b->_repeat_triggered) {
        if (now - b->_last_repeat > b->repeat_period_ms) {
            if (b->_on_repeat != NULL) {
                b->_on_repeat(b->_on_repeat_param);
            }
            b->_last_repeat = now;
        }
    }
    b->_state = state;
}
