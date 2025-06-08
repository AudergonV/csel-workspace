#include "status_led.h"

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define STATUS_LED_GPIO (10) 
#define LED_ON (1)
#define LED_OFF (0)

struct task_struct *led_thread = NULL;
int _blink_frequency = 1;

static int led_control_thread(void *data) {
    while (!kthread_should_stop()) {
        if (_blink_frequency > 0) {
            status_led_on();
            msleep(500 / _blink_frequency); // On for half the period
            status_led_off();
            msleep(500 / _blink_frequency); // Off for half the period
        } else {
            status_led_off(); // If frequency is 0, keep LED off
        }
    }
    return 0;
}

void status_led_init(void) {
    // Initialize the GPIO for the status LED
    if (gpio_request(STATUS_LED_GPIO, "status_led") < 0) {
        pr_err("Failed to request GPIO for status LED\n");
        return;
    }
    if (gpio_direction_output(STATUS_LED_GPIO, 0) < 0) {
        pr_err("Failed to set GPIO direction for status LED\n");
        gpio_free(STATUS_LED_GPIO);
        return;
    }
    led_thread = kthread_run(led_control_thread, NULL, "led_control_thread");
    if (IS_ERR(led_thread)) {
        pr_err("Failed to create LED control thread\n");
        return;
    }
    // Initialize the status LED hardware
    pr_info("Status LED initialized\n");
}

void status_led_deinit(void) {
    if (led_thread) {
        kthread_stop(led_thread);
        led_thread = NULL;
    }
    gpio_free(STATUS_LED_GPIO);
    // Deinitialize the status LED hardware
    pr_info("Status LED deinitialized\n");
}

void status_led_on(void) {
    gpio_set_value(STATUS_LED_GPIO, LED_ON);
}

void status_led_off(void) {
    gpio_set_value(STATUS_LED_GPIO, LED_OFF);
}

void status_led_set_blink_freq(int freq) {
    if (freq < 0) {
        pr_err("Invalid blink frequency: %d. Must be non-negative.\n", freq);
        return;
    }
    _blink_frequency = freq;
    pr_info("Status LED blink frequency set to: %d Hz\n", freq);
}