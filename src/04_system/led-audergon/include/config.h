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

// GPIO configuration
#define LED_GPIO (10)
#define BUTTON_GPIO_K1 (0)
#define BUTTON_GPIO_K2 (2)
#define BUTTON_GPIO_K3 (3)
// App configuration
#define MIN_FREQUENCY_HZ (0.5)
#define INIT_FREQUENCY_HZ (2)
#define MAX_FREQUENCY_HZ (10)
#define BUTTON_POLLING_PERIOD_MS (50)
#define DELTA_MS (100)