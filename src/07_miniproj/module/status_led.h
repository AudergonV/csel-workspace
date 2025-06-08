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
 * Abstract: Status Led Header
 *
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */
#ifndef _STATUS_LED_H__
#define _STATUS_LED_H__

void status_led_init(void);
void status_led_deinit(void);
void status_led_on(void);
void status_led_off(void);
void status_led_set_blink_freq(int freq);

#endif // _STATUS_LED_H__