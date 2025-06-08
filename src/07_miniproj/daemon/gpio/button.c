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
 * Abstract: Button GPIO-based library
 * Button handling library for GPIO-based button input management.
 * Provides functionality to monitor multiple buttons using Linux sysfs
 * GPIO interface with event-driven detection for button presses using epoll.
 *
 * Author:  Bastien Veuthey
 * Date:    07.06.2025
 */

#include "button.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

/**
 * Initialize button context structure
 * Clears all button data and resets counters to prepare for button registration
 * @param ctx Pointer to button context to initialize
 * @return 0 on success, -1 on error (null pointer)
 */
int button_ctx_init(button_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }
    
    memset(ctx, 0, sizeof(*ctx));
    return 0;
}

/**
 * Set callback functions for button events
 * @param ctx Button context
 * @param press_cb Callback for button press events (can be NULL)
 * @param release_cb Callback for button release events (can be NULL)
 */
void button_set_callbacks(button_ctx_t *ctx, button_callback_t press_cb, button_callback_t release_cb)
{
    if (!ctx) {
        return;
    }
    
    ctx->press_callback = press_cb;
    ctx->release_callback = release_cb;
}

/**
 * Set user data passed to callback functions
 * @param ctx Button context
 * @param user_data User data pointer
 */
void button_set_user_data(button_ctx_t *ctx, void *user_data)
{
    if (!ctx) {
        return;
    }
    
    ctx->user_data = user_data;
}

/**
 * Add a button to the monitoring context
 * Exports GPIO pin, configures it for input with both edge detection,
 * and adds it to the button context for polling
 * @param ctx Button context to add button to
 * @param pin GPIO pin number for the button
 * @param name Human-readable name for the button (e.g., "K1", "K2")
 * @return 0 on success, -1 on error
 */
int button_add(button_ctx_t *ctx, uint8_t pin, button_id_t id, const char *name)
{
    if (!ctx || !name || ctx->count >= MAX_BUTTONS) {
        return -1;
    }
    
    if (gpio_export(pin) != GPIO_SUCCESS) {
        return -1;
    }
    
    if (gpio_set_direction(pin, GPIO_DIRECTION_IN) != GPIO_SUCCESS) {
        gpio_unexport(pin);
        return -1;
    }
    
    // Set edge detection to both rising and falling for press/release detection
    if (gpio_set_edge(pin, GPIO_EDGE_BOTH) != GPIO_SUCCESS) {
        gpio_unexport(pin);
        return -1;
    }
    
    int fd = gpio_open_fd(pin);
    if (fd < 0) {
        gpio_unexport(pin);
        return -1;
    }
    
    // Read initial value to clear any pending events and set initial state
    char buf;
    lseek(fd, 0, SEEK_SET);
    read(fd, &buf, 1);
    
    button_t *btn = &ctx->buttons[ctx->count];
    btn->pin = pin;
    btn->name = name;
    btn->id = id;
    btn->fd = fd;
    btn->last_state = (buf == '1') ? 1 : 0;

    ctx->count++;
    return 0;
}

/**
 * Handle a single button event for a specific button index
 * Reads GPIO state and calls appropriate callback if state changed
 * @param ctx Button context
 * @param button_idx Index of the button that had an event
 * @return 1 if event was handled, 0 if no state change, -1 on error
 */
int button_handle_event(button_ctx_t *ctx, size_t button_idx)
{
    if (!ctx || button_idx >= ctx->count) {
        return -1;
    }

    button_t *button = &ctx->buttons[button_idx];

    // Read current GPIO value to clear interrupt
    char value;
    lseek(button->fd, 0, SEEK_SET);
    if (read(button->fd, &value, 1) <= 0) {
        return -1;  // Error reading GPIO value
    }
        int current_state = (value == '1') ? 1 : 0;  // 0=released, 1=pressed (active high)
        
        // Check for state change
        if (current_state != button->last_state) {
            button->last_state = current_state;
            
            // Call appropriate callback
            if (current_state == 0 && ctx->release_callback) {  // Button released
                ctx->release_callback(button, ctx->user_data);
            } else if (current_state == 1 && ctx->press_callback) {  // Button pressed
                ctx->press_callback(button, ctx->user_data);
            }
            return 1; // Event handled
        }

    return 0;  // No state change detected
}

/**
 * Clean up button context and release GPIO resources
 * Closes file descriptors and unexports all GPIO pins
 * @param ctx Button context to clean up
 */
void button_cleanup(button_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }
    
    for (size_t i = 0; i < ctx->count; i++) {
        gpio_close_fd(ctx->buttons[i].fd);
        gpio_unexport(ctx->buttons[i].pin);
    }

    memset(ctx, 0, sizeof(*ctx));
}