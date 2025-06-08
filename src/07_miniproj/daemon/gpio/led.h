/**
 * @file led.h
 * @brief LED control library for GPIO-based LED management
 * 
 * This header provides LED control functions for managing power indication
 * LEDs through GPIO interface with button press signaling capabilities.
 */

#ifndef LED_H
#define LED_H

#include <pthread.h>
#include <stdint.h>

/* LED control state */
typedef struct {
    uint16_t pin;          /* GPIO pin number for LED */
    int initialized;       /* Initialization flag */
    int current_state;     /* Current LED state (0=OFF, 1=ON) */
    int button_count;      /* Number of buttons currently pressed */
    pthread_mutex_t mutex; /* Thread safety mutex */
} led_control_t;

/**
 * @brief Initialize LED control system
 * @param pin GPIO pin number for the LED
 * @return 0 on success, -1 on error
 */
int led_init(uint16_t pin);

/**
 * @brief Set LED state directly
 * @param state 1 for ON, 0 for OFF
 */
void led_set_state(int state);

/**
 * @brief Signal button press with LED (turn on while button held)
 * Call this when a button is pressed down
 */
void led_button_pressed(void);

/**
 * @brief Signal button release with LED (turn off when button released)
 * Call this when a button is released
 */
void led_button_released(void);

/**
 * @brief Get current LED state
 * @return 1 if LED is ON, 0 if OFF
 */
int led_get_state(void);

/**
 * @brief Cleanup LED resources
 */
void led_cleanup(void);

#endif /* LED_H */