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
 * Abstract: GPIO library
 * Linux sysfs GPIO interface library providing low-level GPIO operations.
 * Supports GPIO export/unexport, direction setting, edge detection,
 * and read/write operations through the /sys/class/gpio interface.
 *
 * Author:  Bastien Veuthey
 * Date:    07.06.2025
 */

#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Sysfs paths for GPIO control */
#define GPIO_EXPORT_PATH "/sys/class/gpio/export"     /* Export GPIO pins */
#define GPIO_UNEXPORT_PATH "/sys/class/gpio/unexport" /* Unexport GPIO pins */
#define GPIO_BASE_PATH "/sys/class/gpio/gpio%d"       /* Base path for GPIO operations */

/* String mappings for GPIO direction values */
static const char *gpio_direction_str[] = {
    [GPIO_DIRECTION_IN] = "in",   /* Input direction */
    [GPIO_DIRECTION_OUT] = "out"  /* Output direction */
};

/* String mappings for GPIO edge detection values */
static const char *gpio_edge_str[] = {
    [GPIO_EDGE_NONE] = "none",       /* No edge detection */
    [GPIO_EDGE_RISING] = "rising",   /* Rising edge detection */
    [GPIO_EDGE_FALLING] = "falling", /* Falling edge detection */
    [GPIO_EDGE_BOTH] = "both"        /* Both edge detection */
};

/**
 * Export a GPIO pin to userspace
 * Makes the GPIO pin available for control through sysfs interface
 * @param pin GPIO pin number to export
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_export(uint8_t pin)
{
    char buf[16];
    int fd = open(GPIO_EXPORT_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open GPIO export");
        return GPIO_ERROR;
    }
    
    int len = snprintf(buf, sizeof(buf), "%d", pin);
    // Write pin number to export file
    if (write(fd, buf, len) < 0 && errno != EBUSY) {
        perror("Failed to export GPIO");
        close(fd);
        return GPIO_ERROR;
    }
    
    close(fd);
    return GPIO_SUCCESS;
}

/**
 * Unexport a GPIO pin from userspace
 * Removes the GPIO pin from sysfs control, freeing the resource
 * @param pin GPIO pin number to unexport
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_unexport(uint8_t pin)
{
    char buf[16];
    int fd = open(GPIO_UNEXPORT_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open GPIO unexport");
        return GPIO_ERROR;
    }
    
    int len = snprintf(buf, sizeof(buf), "%d", pin);
    if (write(fd, buf, len) < 0) {
        perror("Failed to unexport GPIO");
        close(fd);
        return GPIO_ERROR;
    }
    
    close(fd);
    return GPIO_SUCCESS;
}

/**
 * Set GPIO pin direction (input or output)
 * Configures the GPIO pin for either reading (input) or writing (output)
 * @param pin GPIO pin number
 * @param direction GPIO_DIRECTION_IN or GPIO_DIRECTION_OUT
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_set_direction(uint8_t pin, gpio_direction_t direction)
{
    char path[64];
    // Build path to direction file for this GPIO pin
    snprintf(path, sizeof(path), GPIO_BASE_PATH "/direction", pin);
    
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open GPIO direction");
        return GPIO_ERROR;
    }
    
    const char *dir_str = gpio_direction_str[direction];
    if (write(fd, dir_str, strlen(dir_str)) < 0) {
        perror("Failed to set GPIO direction");
        close(fd);
        return GPIO_ERROR;
    }
    
    close(fd);
    return GPIO_SUCCESS;
}

/**
 * Set GPIO edge detection mode
 * Configures interrupt generation for specified edge transitions
 * @param pin GPIO pin number
 * @param edge Edge detection type (none, rising, falling, both)
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_set_edge(uint8_t pin, gpio_edge_t edge)
{
    char path[64];
    // Build path to edge file for this GPIO pin
    snprintf(path, sizeof(path), GPIO_BASE_PATH "/edge", pin);
    
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open GPIO edge");
        return GPIO_ERROR;
    }
    
    const char *edge_str = gpio_edge_str[edge];
    if (write(fd, edge_str, strlen(edge_str)) < 0) {
        perror("Failed to set GPIO edge");
        close(fd);
        return GPIO_ERROR;
    }
    
    close(fd);
    return GPIO_SUCCESS;
}

/**
 * Write a value to GPIO output pin
 * Sets the GPIO pin to high (1) or low (0) state
 * @param pin GPIO pin number (must be configured as output)
 * @param value GPIO_VALUE_HIGH or GPIO_VALUE_LOW
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_write(uint8_t pin, gpio_value_t value)
{
    char path[64];
    // Build path to value file for this GPIO pin
    snprintf(path, sizeof(path), GPIO_BASE_PATH "/value", pin);
    
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open GPIO value for write");
        return GPIO_ERROR;
    }
    
    // Convert GPIO value to character representation
    char val_char = (value == GPIO_VALUE_HIGH) ? '1' : '0';
    if (write(fd, &val_char, 1) < 0) {
        perror("Failed to write GPIO value");
        close(fd);
        return GPIO_ERROR;
    }
    
    close(fd);
    return GPIO_SUCCESS;
}

/**
 * Read the current value of a GPIO pin
 * Returns the current state of the GPIO pin (high or low)
 * @param pin GPIO pin number
 * @param value Pointer to store the read value
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_read(uint8_t pin, gpio_value_t *value)
{
    char path[64];
    // Build path to value file for this GPIO pin
    snprintf(path, sizeof(path), GPIO_BASE_PATH "/value", pin);
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open GPIO value for read");
        return GPIO_ERROR;
    }
    
    char val_char;
    if (read(fd, &val_char, 1) < 0) {
        perror("Failed to read GPIO value");
        close(fd);
        return GPIO_ERROR;
    }
    
    // Convert character to GPIO value
    *value = (val_char == '1') ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW;
    close(fd);
    return GPIO_SUCCESS;
}

/**
 * Open file descriptor for GPIO value file
 * Opens the GPIO value file for direct reading, useful for polling
 * @param pin GPIO pin number
 * @return File descriptor on success, -1 on error
 */
int gpio_open_fd(uint8_t pin)
{
    char path[64];
    snprintf(path, sizeof(path), GPIO_BASE_PATH "/value", pin);
    // Open in non-blocking mode for polling
    return open(path, O_RDONLY | O_NONBLOCK);
}

/**
 * Close GPIO file descriptor
 * Safely closes a GPIO file descriptor if it's valid
 * @param fd File descriptor to close
 */
void gpio_close_fd(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}