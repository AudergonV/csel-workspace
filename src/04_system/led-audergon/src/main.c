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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <time.h>
#include "led.h"
#include "periodic_timer.h"

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */

 #define LED "10"


int main(void)
{
    int led = open_led(LED);
    led_toggle(led, true);


    periodic_timer_t mytimer;
    if (periodic_timer_init(&mytimer, 100) == -1) {
        perror("ERROR");
    }
    

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = mytimer._tfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, mytimer._tfd, &ev) == -1) {
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

        // On lit le timerfd pour réarmer le timer
        uint64_t expirations;
        read(mytimer._tfd, &expirations, sizeof(expirations));
        led_toggle(led, led_state);
        led_state = !led_state;
        periodic_timer_increase_period(&mytimer, 50);
    }
    
    return 0;
}