#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "gpio.h"
#include "button.h"

#define LED_PIN 10
#define K1_PIN 0
#define K2_PIN 2
#define K3_PIN 3

static volatile int running = 1;
static int led_state = 0;

static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

static void button_pressed(const button_t *button, void *user_data)
{
    (void)user_data;
    printf("%s pressed\n", button->name);
    led_state = !led_state;
    gpio_write(LED_PIN, led_state ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW);
}

int main(void)
{
    button_ctx_t button_ctx;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (button_ctx_init(&button_ctx) < 0) {
        fprintf(stderr, "Failed to initialize button context\n");
        return EXIT_FAILURE;
    }
    
    if (button_add(&button_ctx, K1_PIN, "K1") < 0 ||
        button_add(&button_ctx, K2_PIN, "K2") < 0 ||
        button_add(&button_ctx, K3_PIN, "K3") < 0) {
        fprintf(stderr, "Failed to add buttons\n");
        button_cleanup(&button_ctx);
        return EXIT_FAILURE;
    }
    
    if (gpio_export(LED_PIN) != GPIO_SUCCESS ||
        gpio_set_direction(LED_PIN, GPIO_DIRECTION_OUT) != GPIO_SUCCESS ||
        gpio_write(LED_PIN, GPIO_VALUE_LOW) != GPIO_SUCCESS) {
        fprintf(stderr, "Failed to initialize LED\n");
        button_cleanup(&button_ctx);
        return EXIT_FAILURE;
    }
    
    printf("Button daemon-like started (PID: %d)\n", getpid());
    printf("Press Ctrl+C to stop\n");
    
    while (running) {
        int ret = button_poll(&button_ctx, 1000);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("poll failed");
            break;
        }
        
        if (ret > 0) {
            button_handle_events(&button_ctx, button_pressed, NULL);
        }
    }
    
    printf("\nShutting down daemon-like application...\n");
    gpio_write(LED_PIN, GPIO_VALUE_LOW);
    gpio_unexport(LED_PIN);
    button_cleanup(&button_ctx);
    
    return EXIT_SUCCESS;
}

