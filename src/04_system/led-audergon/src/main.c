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
 * Date:    11.04.2025
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "button.h"
#include "config.h"
#include "convert.h"
#include "gpio.h"
#include "periodic_timer.h"

void increase_period(void* param)
{
    periodic_timer_t* t = (periodic_timer_t*)param;
    if (periodic_timer_increase_period(t, DELTA_MS) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
}

void decrease_period(void* param)
{
    periodic_timer_t* t = (periodic_timer_t*)param;
    if (periodic_timer_decrease_period(t, DELTA_MS) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
}

void reset_period(void* param)
{
    periodic_timer_t* t = (periodic_timer_t*)param;
    if (periodic_timer_reset_period(t) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    gpio_t led;
    button_t button_left;
    button_t button_middle;
    button_t button_right;
    periodic_timer_t ledtimer;
    periodic_timer_t buttontimer;  // Poll button state to detect repeat
    // Initialize the buttons and led
    if (gpio_init(&led, LED_GPIO, GPIO_DIR_OUT) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    if (button_init(&button_left, BUTTON_GPIO_K1, 1000, 100) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    if (button_init(&button_middle, BUTTON_GPIO_K2, 1000, 100) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    if (button_init(&button_right, BUTTON_GPIO_K3, 1000, 100) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    // Initialize the timers
    long period_ms     = SEC_TO_MS_FACTOR / INIT_FREQUENCY_HZ;
    long min_period_ms = SEC_TO_MS_FACTOR / MAX_FREQUENCY_HZ;
    long max_period_ms = SEC_TO_MS_FACTOR / MIN_FREQUENCY_HZ;
    if (periodic_timer_init(
            &ledtimer, period_ms, min_period_ms, max_period_ms) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    if (periodic_timer_init(&buttontimer,
                            BUTTON_POLLING_PERIOD_MS,
                            BUTTON_POLLING_PERIOD_MS,
                            BUTTON_POLLING_PERIOD_MS) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    // Attach the button callbacks
    button_attach_on_click(&button_left, increase_period, &ledtimer);
    button_attach_on_click(&button_middle, reset_period, &ledtimer);
    button_attach_on_click(&button_right, decrease_period, &ledtimer);
    button_attach_on_repeat(&button_left, increase_period, &ledtimer);
    button_attach_on_repeat(&button_right, decrease_period, &ledtimer);

    // epoll
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events  = EPOLLIN;
    ev.data.fd = ledtimer._tfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, ledtimer._tfd, &ev) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    ev.data.fd = buttontimer._tfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, buttontimer._tfd, &ev) == -1) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    bool led_state = false;

    while (1) {
        struct epoll_event events[1];
        int nfds = epoll_wait(epfd, events, 1, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        if (events[0].data.fd == buttontimer._tfd) {
            uint64_t expirations;
            read(buttontimer._tfd, &expirations, sizeof(expirations));
            button_update(&button_left);
            button_update(&button_middle);
            button_update(&button_right);
            continue;
        }
        if (events[0].data.fd == ledtimer._tfd) {
            uint64_t expirations;
            read(ledtimer._tfd, &expirations, sizeof(expirations));
            gpio_write(&led, led_state);
            led_state = !led_state;
        }
    }
    return 0;
}