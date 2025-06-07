#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum {
    GPIO_SUCCESS = 0,
    GPIO_ERROR = -1
} gpio_result_t;

typedef enum {
    GPIO_DIRECTION_IN,
    GPIO_DIRECTION_OUT
} gpio_direction_t;

typedef enum {
    GPIO_EDGE_NONE,
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH
} gpio_edge_t;

typedef enum {
    GPIO_VALUE_LOW = 0,
    GPIO_VALUE_HIGH = 1
} gpio_value_t;

gpio_result_t gpio_export(uint8_t pin);
gpio_result_t gpio_unexport(uint8_t pin);
gpio_result_t gpio_set_direction(uint8_t pin, gpio_direction_t direction);
gpio_result_t gpio_set_edge(uint8_t pin, gpio_edge_t edge);
gpio_result_t gpio_write(uint8_t pin, gpio_value_t value);
gpio_result_t gpio_read(uint8_t pin, gpio_value_t *value);
int gpio_open_fd(uint8_t pin);
void gpio_close_fd(int fd);

#endif /* GPIO_H */