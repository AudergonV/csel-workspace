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
 * Date:    11.04.2025
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "gpio.h"

typedef struct button {
    gpio_t gpio_ro;
    unsigned int press_time_until_repeat_ms;
    unsigned int repeat_period_ms;
    uint64_t _last_change;
    uint64_t _last_repeat;
    bool _state;
    bool _repeat_triggered;
    void (*_on_click)(void*);
    void (*_on_repeat)(void*);
    void* _on_click_param;
    void* _on_repeat_param;
} button_t;

/**
 * Initializes a button with the given GPIO pin number and press time until
 * repeat.
 *
 * @param b a pointer to a button_t structure
 * @param gpio the GPIO pin number
 * @param press_time_until_repeat_ms the time in milliseconds until repeat
 * @param repeat_period_ms the repeat period in milliseconds
 *
 * @return -1 if an error occurred
 */
int button_init(button_t* b,
                int gpio,
                unsigned int press_time_until_repeat_ms,
                unsigned int repeat_period_ms);

/**
 * Attaches a function to be called when the button is clicked.
 *
 * @param b a pointer to a button_t structure
 * @param func the function to be called on click
 * @param param the parameter to be passed to the function
 */
void button_attach_on_click(button_t* b, void (*func)(void*), void* param);

/**
 * Attaches a function to be called repeatedly while the button is pressed.
 *
 * @param b a pointer to a button_t structure
 * @param func the function to be called on repeat
 * @param param the parameter to be passed to the function
 */
void button_attach_on_repeat(button_t* b, void (*func)(void*), void* param);

/**
 * Updates the button state and triggers the appropriate actions.
 *
 * @param b a pointer to a button_t structure
 */
void button_update(button_t* b);

#endif  // BUTTON_H_