/**
 * @file button.h
 * @brief Button handling library for GPIO-based button input management
 * 
 * This header defines structures and functions for monitoring multiple buttons
 * using Linux sysfs GPIO interface with poll-based event detection.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stddef.h>
#include <sys/epoll.h>

/** Maximum number of buttons that can be monitored simultaneously */
#define MAX_BUTTONS 3

typedef enum {
    BUTTON_K1 = 0,
    BUTTON_K2,
    BUTTON_K3
} button_id_t;

typedef struct {
    uint8_t pin;      /* GPIO pin number */
    button_id_t id;       /* Logical identifier */
    const char *name; /* Human-readable button name */
    int fd;           /* File descriptor for GPIO value file */
    int last_state;   /* Last known button state (0=pressed, 1=released) */
} button_t;

/**
 * @brief Callback function type for button press events
 * @param button Pointer to the button that was pressed
 * @param user_data User-provided data passed to the callback
 */
typedef void (*button_callback_t)(const button_t *button, void *user_data);

/**
 * @brief Button monitoring context
 * Contains all buttons and polling structures for event detection
 */
typedef struct {
    button_t buttons[MAX_BUTTONS];      /* Array of button structures */
    size_t count;                       /* Number of active buttons */
    button_callback_t press_callback;   /* Callback for button press events */
    button_callback_t release_callback; /* Callback for button release events */
    void *user_data;                    /* User data passed to callbacks */
} button_ctx_t;

/**
 * @brief Initialize button context structure
 * @param ctx Pointer to button context to initialize
 * @return 0 on success, -1 on error
 */
int button_ctx_init(button_ctx_t *ctx);

/**
 * @brief Set callback functions for button events
 * @param ctx Button context
 * @param press_cb Callback for button press events (can be NULL)
 * @param release_cb Callback for button release events (can be NULL)
 */
void button_set_callbacks(button_ctx_t *ctx, button_callback_t press_cb, button_callback_t release_cb);

/**
 * @brief Set user data passed to callback functions
 * @param ctx Button context
 * @param user_data User data pointer
 */
void button_set_user_data(button_ctx_t *ctx, void *user_data);

/**
 * @brief Add a button to the monitoring context
 * @param ctx Button context to add button to
 * @param pin GPIO pin number for the button
 * @param name Human-readable name for the button
 * @return 0 on success, -1 on error
 */
int button_add(button_ctx_t *ctx, uint8_t pin, button_id_t id, const char *name);

/**
 * @brief Handle a single button event for a specific button index
 * @param ctx Button context
 * @param button_idx Index of the button that had an event
 * @return 1 if event was handled, 0 if no state change, -1 on error
 */
int button_handle_event(button_ctx_t *ctx, size_t button_idx);

/**
 * @brief Clean up button context and release GPIO resources
 * @param ctx Button context to clean up
 */
void button_cleanup(button_ctx_t *ctx);

#endif /* BUTTON_H */