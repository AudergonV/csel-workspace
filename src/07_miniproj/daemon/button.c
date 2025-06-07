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
 * GPIO interface with poll-based event detection for button presses.
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
 * Add a button to the monitoring context
 * Exports GPIO pin, configures it for input with falling edge detection,
 * and adds it to the button context for polling
 * @param ctx Button context to add button to
 * @param pin GPIO pin number for the button
 * @param name Human-readable name for the button (e.g., "K1", "K2")
 * @return 0 on success, -1 on error
 */
int button_add(button_ctx_t *ctx, uint8_t pin, const char *name)
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
    
    if (gpio_set_edge(pin, GPIO_EDGE_FALLING) != GPIO_SUCCESS) {
        gpio_unexport(pin);
        return -1;
    }
    
    int fd = gpio_open_fd(pin);
    if (fd < 0) {
        gpio_unexport(pin);
        return -1;
    }
    
    // Read initial value to clear any pending events
    char buf;
    lseek(fd, 0, SEEK_SET);
    read(fd, &buf, 1);
    
    button_t *btn = &ctx->buttons[ctx->count];
    btn->pin = pin;
    btn->name = name;
    btn->fd = fd;
    
    ctx->pfds[ctx->count].fd = fd;
    ctx->pfds[ctx->count].events = POLLPRI | POLLERR;
    
    ctx->count++;
    return 0;
}

/**
 * Poll for button events using the poll() system call
 * Waits for button press events on all registered buttons
 * @param ctx Button context containing buttons to monitor
 * @param timeout_ms Timeout in milliseconds (-1 for infinite wait)
 * @return Number of file descriptors with events, 0 on timeout, -1 on error
 */
int button_poll(button_ctx_t *ctx, int timeout_ms)
{
    if (!ctx || ctx->count == 0) {
        return -1;
    }
    
    return poll(ctx->pfds, ctx->count, timeout_ms);
}

/**
 * Process button events after poll() indicates activity
 * Reads button states and calls the provided callback for each pressed button
 * @param ctx Button context with pending events
 * @param callback Function to call when button is pressed
 * @param user_data User data to pass to callback function
 * @return Number of events handled, -1 on error
 */
int button_handle_events(button_ctx_t *ctx, button_callback_t callback, void *user_data)
{
    if (!ctx || !callback) {
        return -1;
    }
    
    int events_handled = 0;
    
    // Check each button for events
    for (size_t i = 0; i < ctx->count; i++) {
        if (ctx->pfds[i].revents & (POLLPRI | POLLERR)) {
            // Read the GPIO value to acknowledge the event
            lseek(ctx->pfds[i].fd, 0, SEEK_SET);
            char val;
            read(ctx->pfds[i].fd, &val, 1);
            
            // Call the callback function for this button press
            callback(&ctx->buttons[i], user_data);
            events_handled++;
        }
    }
    
    return events_handled;
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