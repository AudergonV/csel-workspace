#include "status_led.h"

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define LED_ON 1
#define LED_OFF 0
#define HALF_SECOND_MS 500U

struct task_struct *led_thread = NULL;
unsigned int _blink_frequency = 0;

static int led_control_thread(void *data) {
    while (!kthread_should_stop()) {
        if (_blink_frequency > 0) {
            status_led_on();
            msleep(HALF_SECOND_MS / _blink_frequency); // On for half the period
            status_led_off();
            msleep(HALF_SECOND_MS / _blink_frequency); // Off for half the period
        } else {
            status_led_off();
        }
    }
    return 0;
}

int status_led_init(void) {
    // Initialize the GPIO for the status LED
    int status = gpio_request(CSEL_STATUS_LED_GPIO, "status_led");
    if (status < 0) {
        pr_err("failed to request GPIO for status LED\n");
        return status;
    }
    status = gpio_direction_output(CSEL_STATUS_LED_GPIO, 0);
    if (status < 0) {
        pr_err("failed to set GPIO direction for status LED\n");
        gpio_free(CSEL_STATUS_LED_GPIO);
        return status;
    }
    led_thread = kthread_run(led_control_thread, NULL, "led_control_thread");
    if (IS_ERR(led_thread)) {
        pr_err("failed to create LED control thread\n");
        return PTR_ERR(led_thread);
    }
    // Initialize the status LED hardware
    pr_info("status LED initialized\n");
    return 0;
}

void status_led_deinit(void) {
    if (led_thread) {
        kthread_stop(led_thread);
        led_thread = NULL;
    }
    gpio_free(CSEL_STATUS_LED_GPIO);
    pr_info("status LED deinitialized\n");
}

void status_led_on(void) {
    gpio_set_value(CSEL_STATUS_LED_GPIO, LED_ON);
}

void status_led_off(void) {
    gpio_set_value(CSEL_STATUS_LED_GPIO, LED_OFF);
}

void status_led_set_blink_freq(unsigned int freq) {
    _blink_frequency = freq;
}