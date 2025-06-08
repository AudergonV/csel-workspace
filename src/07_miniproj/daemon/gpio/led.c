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
 * Abstract: LED control library
 * LED control library for GPIO-based LED management with button press
 * signaling capabilities. Provides simple interface for LED state control
 * and button press indication through LED feedback.
 *
 * Author:  Bastien Veuthey
 * Date:    07.06.2025
 */

#include "led.h"
#include "gpio.h"
#include <pthread.h>
#include <syslog.h>

static led_control_t led_ctrl = {0};

/**
 * Initialize LED control system
 * Sets up GPIO pin for LED control and initializes LED to OFF state
 * @param pin GPIO pin number for the LED
 * @return 0 on success, -1 on error
 */
int led_init(uint16_t pin)
{
    if (led_ctrl.initialized) {
        syslog(LOG_WARNING, "LED already initialized");
        return 0;
    }
    
    // Export GPIO pin
    if (gpio_export(pin) != GPIO_SUCCESS) {
        syslog(LOG_ERR, "Failed to export LED GPIO pin %d", pin);
        return -1;
    }
    
    // Set direction to output
    if (gpio_set_direction(pin, GPIO_DIRECTION_OUT) != GPIO_SUCCESS) {
        syslog(LOG_ERR, "Failed to set LED GPIO direction");
        gpio_unexport(pin);
        return -1;
    }
    
    // Initialize LED to OFF state
    if (gpio_write(pin, GPIO_VALUE_LOW) != GPIO_SUCCESS) {
        syslog(LOG_ERR, "Failed to initialize LED state");
        gpio_unexport(pin);
        return -1;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&led_ctrl.mutex, NULL) != 0) {
        syslog(LOG_ERR, "Failed to initialize LED mutex");
        gpio_unexport(pin);
        return -1;
    }
    
    led_ctrl.pin = pin;
    led_ctrl.current_state = 0;
    led_ctrl.button_count = 0;
    led_ctrl.initialized = 1;
    
    return 0;
}

/**
 * Set LED state directly
 * @param state 1 for ON, 0 for OFF
 */
void led_set_state(int state)
{
    if (!led_ctrl.initialized) {
        return;
    }
    
    pthread_mutex_lock(&led_ctrl.mutex);
    
    gpio_value_t gpio_val = state ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW;
    if (gpio_write(led_ctrl.pin, gpio_val) == GPIO_SUCCESS) {
        led_ctrl.current_state = state;
    }
    
    pthread_mutex_unlock(&led_ctrl.mutex);
}

/**
 * Signal button press with LED (turn on while button held)
 * Call this when a button is pressed down
 */
void led_button_pressed(void)
{
    if (!led_ctrl.initialized) {
        return;
    }
    
    pthread_mutex_lock(&led_ctrl.mutex);
    
    led_ctrl.button_count++;
    
    // Turn LED on if this is the first button pressed
    if (led_ctrl.button_count == 1) {
        if (gpio_write(led_ctrl.pin, GPIO_VALUE_HIGH) == GPIO_SUCCESS) {
            led_ctrl.current_state = 1;
        }
    }
    
    pthread_mutex_unlock(&led_ctrl.mutex);
}

/**
 * Signal button release with LED (turn off when button released)
 * Call this when a button is released
 */
void led_button_released(void)
{
    if (!led_ctrl.initialized) {
        return;
    }
    
    pthread_mutex_lock(&led_ctrl.mutex);
    
    if (led_ctrl.button_count > 0) {
        led_ctrl.button_count--;
    }
    
    // Turn LED off if no buttons are pressed
    if (led_ctrl.button_count == 0) {
        if (gpio_write(led_ctrl.pin, GPIO_VALUE_LOW) == GPIO_SUCCESS) {
            led_ctrl.current_state = 0;
        }
    }
    
    pthread_mutex_unlock(&led_ctrl.mutex);
}

/**
 * Get current LED state
 * @return 1 if LED is ON, 0 if OFF
 */
int led_get_state(void)
{
    if (!led_ctrl.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&led_ctrl.mutex);
    int state = led_ctrl.current_state;
    pthread_mutex_unlock(&led_ctrl.mutex);
    
    return state;
}

/**
 * Cleanup LED resources
 * Turns off LED and releases GPIO resources
 */
void led_cleanup(void)
{
    if (!led_ctrl.initialized) {
        return;
    }
    
    pthread_mutex_lock(&led_ctrl.mutex);
    
    // Turn LED off
    gpio_write(led_ctrl.pin, GPIO_VALUE_LOW);
    
    // Unexport GPIO
    gpio_unexport(led_ctrl.pin);
    
    led_ctrl.initialized = 0;
    led_ctrl.current_state = 0;
    led_ctrl.button_count = 0;
    
    pthread_mutex_unlock(&led_ctrl.mutex);
    pthread_mutex_destroy(&led_ctrl.mutex);
}