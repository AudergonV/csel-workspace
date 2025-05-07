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

#include "gpio.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_BASE "/sys/class/gpio/gpio"
#define GPIO_DIR "/direction"
#define GPIO_VALUE "/value"
#define ON_STR "1"
#define OFF_STR "0"
#define DIR_OUT_STR "out"
#define DIR_OUT_STR_LEN 3
#define DIR_IN_STR "in"
#define DIR_IN_STR_LEN 2

int gpio_init(gpio_t* g, unsigned int gpio, gpio_dir_t dir)
{
    g->fd_ro = -1;
    g->gpio  = gpio;
    g->_dir  = dir;
    memset(g->_gpio_str, 0, sizeof(g->_gpio_str));
    memset(g->_gpio_path, 0, sizeof(g->_gpio_path));
    memset(g->_gpio_dir, 0, sizeof(g->_gpio_dir));
    memset(g->_gpio_value, 0, sizeof(g->_gpio_value));
    snprintf(g->_gpio_str, sizeof(g->_gpio_str), "%u", gpio);
    // export pin to sysfs
    int f = open(GPIO_EXPORT, O_WRONLY);
    if (f == -1) {
        perror("ERROR: open GPIO_EXPORT");
        return -1;
    }
    write(f, g->_gpio_str, strlen(g->_gpio_str));
    close(f);

    strncpy(g->_gpio_path, GPIO_BASE, sizeof(g->_gpio_path));
    strncat(g->_gpio_path,
            g->_gpio_str,
            strnlen(g->_gpio_str, sizeof(g->_gpio_str)));
    strncpy(g->_gpio_dir,
            g->_gpio_path,
            strnlen(g->_gpio_path, sizeof(g->_gpio_dir)));
    strncat(g->_gpio_dir,
            GPIO_DIR,
            sizeof(g->_gpio_dir) - strlen(g->_gpio_dir) - 1);
    strncpy(g->_gpio_value,
            g->_gpio_path,
            strnlen(g->_gpio_path, sizeof(g->_gpio_value)));
    strncat(g->_gpio_value,
            GPIO_VALUE,
            sizeof(g->_gpio_value) - strlen(g->_gpio_value) - 1);

    // config pin
    f = open(g->_gpio_dir, O_WRONLY);
    if (f == -1) {
        perror("ERROR: open GPIO_DIR");
        return -1;
    }
    if (dir == GPIO_DIR_IN) {
        write(f, DIR_IN_STR, DIR_IN_STR_LEN);
    } else if (dir == GPIO_DIR_OUT) {
        write(f, DIR_OUT_STR, DIR_OUT_STR_LEN);
    } else {
        close(f);
        return -1;
    }
    close(f);

    // open gpio value attribute
    f        = open(g->_gpio_value, O_RDWR);
    g->fd_ro = f;
    return f;
}

void gpio_close(gpio_t* g)
{
    // unexport pin out of sysfs
    close(g->fd_ro);
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, g->_gpio_str, strlen(g->_gpio_str));
    close(f);
}

int gpio_write(gpio_t* g, bool value)
{
    if (g->_dir != GPIO_DIR_OUT) {
        return -1;
    }
    int fd = g->fd_ro;
    if (value) {
        pwrite(fd, ON_STR, sizeof(ON_STR), 0);
    } else {
        pwrite(fd, OFF_STR, sizeof(OFF_STR), 0);
    }
    return 0;
}

bool gpio_read(gpio_t* g)
{
    if (g->_dir != GPIO_DIR_IN) {
        return false;
    }
    char value[2];
    int fd = g->fd_ro;
    lseek(fd, 0, SEEK_SET);
    read(fd, value, sizeof(value));
    return (value[0] == '1');
}