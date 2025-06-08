/**
 * @file gpio.h
 * @brief Linux sysfs GPIO interface library
 * 
 * This header provides low-level GPIO operations through the Linux sysfs
 * interface. Supports GPIO export/unexport, direction setting, edge detection,
 * and read/write operations.
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/**
 * @brief GPIO operation result codes
 */
typedef enum {
    GPIO_SUCCESS = 0,   /* Operation successful */
    GPIO_ERROR = -1     /* Operation failed */
} gpio_result_t;

/**
 * @brief GPIO pin direction settings
 */
typedef enum {
    GPIO_DIRECTION_IN,  /* Configure as input pin */
    GPIO_DIRECTION_OUT  /* Configure as output pin */
} gpio_direction_t;

/**
 * @brief GPIO edge detection modes
 */
typedef enum {
    GPIO_EDGE_NONE,     /* No edge detection */
    GPIO_EDGE_RISING,   /* Rising edge detection */
    GPIO_EDGE_FALLING,  /* Falling edge detection */
    GPIO_EDGE_BOTH      /* Both rising and falling edge detection */
} gpio_edge_t;

/**
 * @brief GPIO pin values
 */
typedef enum {
    GPIO_VALUE_LOW = 0,  /* Logic low (0V) */
    GPIO_VALUE_HIGH = 1  /* Logic high (3.3V/5V) */
} gpio_value_t;

/**
 * @brief Export a GPIO pin to userspace
 * @param pin GPIO pin number to export
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_export(uint16_t pin);

/**
 * @brief Unexport a GPIO pin from userspace
 * @param pin GPIO pin number to unexport
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_unexport(uint16_t pin);

/**
 * @brief Set GPIO pin direction (input or output)
 * @param pin GPIO pin number
 * @param direction GPIO_DIRECTION_IN or GPIO_DIRECTION_OUT
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_set_direction(uint16_t pin, gpio_direction_t direction);

/**
 * @brief Set GPIO edge detection mode
 * @param pin GPIO pin number
 * @param edge Edge detection type
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_set_edge(uint16_t pin, gpio_edge_t edge);

/**
 * @brief Write a value to GPIO output pin
 * @param pin GPIO pin number (must be configured as output)
 * @param value GPIO_VALUE_HIGH or GPIO_VALUE_LOW
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_write(uint16_t pin, gpio_value_t value);

/**
 * @brief Read the current value of a GPIO pin
 * @param pin GPIO pin number
 * @param value Pointer to store the read value
 * @return GPIO_SUCCESS on success, GPIO_ERROR on failure
 */
gpio_result_t gpio_read(uint16_t pin, gpio_value_t *value);

/**
 * @brief Open file descriptor for GPIO value file
 * @param pin GPIO pin number
 * @return File descriptor on success, -1 on error
 */
int gpio_open_fd(uint16_t pin);

/**
 * @brief Close GPIO file descriptor
 * @param fd File descriptor to close
 */
void gpio_close_fd(int fd);

#endif /* GPIO_H */