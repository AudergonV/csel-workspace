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

#ifndef TIMER_H_
#define TIMER_H_

#include <sys/timerfd.h>

typedef struct periodic_timer {
    int _tfd;
    struct itimerspec _spec;
    long _initial_period_ms;
    long _min_period_ms;
    long _max_period_ms;
    long _current_period_ms;
} periodic_timer_t;

/**
 * Initialize and start a timer with the given period is ms.
 *
 * @param t a pointer to a periodic_timer_t structure
 * @param period_ms the initial timer period is ms
 * @param min_period_ms the minimum timer period is ms
 * @param max_period_ms the maximum timer period is ms
 *
 * @return -1 if an error occured
 */
int periodic_timer_init(periodic_timer_t* t,
                        long period_ms,
                        long min_period_ms,
                        long max_period_ms);

/**
 * Set a timer's period and apply the change immediately
 *
 * If the new period is greater than the maximum period, the timer is set to the
 * maximum period. If the new period is less than the minimum period, the timer
 * is set to the minimum period.
 *
 * @param t a pointer to a periodic_timer_t structure
 * @param period_ms the new timer period is ms
 *
 * @return -1 if an error occured
 */
int periodic_timer_set_period(periodic_timer_t* t, long period_ms);

/**
 * Increase a timer's period and apply the change immediately
 *
 * new period = old period + delta
 *
 * If the new period is greater than the maximum period, the timer is set to the
 * maximum period.
 *
 * @param t a pointer to a periodic_timer_t structure
 * @param period_ms the delta period is ms
 *
 * @return -1 if an error occured
 */
int periodic_timer_increase_period(periodic_timer_t* t, long delta_ms);

/**
 * Decrease a timer's period and apply the change immediately
 *
 * new period = old period - delta
 *
 * If the new period is less than the minimum period, the timer is set to the
 * minimum period.
 *
 * @param t a pointer to a periodic_timer_t structure
 * @param period_ms the delta period is ms
 *
 * @return -1 if an error occured
 */
int periodic_timer_decrease_period(periodic_timer_t* t, long delta_ms);

/**
 * Reset a timer's period to its initial value and apply the change immediately
 *
 *
 * @param t a pointer to a periodic_timer_t structure
 *
 * @return -1 if an error occured
 */
int periodic_timer_reset_period(periodic_timer_t* t);

#endif  // TIMER_H_