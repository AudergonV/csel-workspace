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

#ifndef GPIO_H_
#define GPIO_H_

#include <stdbool.h>

typedef enum { GPIO_DIR_IN, GPIO_DIR_OUT } gpio_dir_t;

typedef struct gpio {
    int fd_ro;
    unsigned int gpio;
    gpio_dir_t _dir;
    char _gpio_str[16];
    char _gpio_path[64];
    char _gpio_dir[64];
    char _gpio_value[64];
} gpio_t;

/**
 * Init a GPIO by opening the corresponding GPIO file descriptor.
 * PRE: gpio is not initialized
 * POST: gpio is initialized, usable
 * @return -1 if an error occured, otherwise its file descriptor
 */
int gpio_init(gpio_t* g, unsigned int gpio, gpio_dir_t dir);

/**
 * Closes the GPIO.
 * PRE: gpio is initialized
 * POST: gpio is closed, unusable after this call
 * @param g a pointer to a gpio_t structure
 */
void gpio_close(gpio_t* g);

/**
 * Changes the status of a GPIO
 * PRE: gpio is initialized
 * @param g a pointer to a gpio_t structure
 * @param value the new status of the GPIO (on = true, off = false)
 */
int gpio_write(gpio_t* g, bool value);

/**
 * Reads the status of a GPIO
 * PRE: gpio is initialized
 * @param g a pointer to a gpio_t structure
 * @return the status of the GPIO (on = true, off = false)
 */
bool gpio_read(gpio_t* g);

#endif  // GPIO_H_