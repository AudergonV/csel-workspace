#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stddef.h>
#include <poll.h>

#define MAX_BUTTONS 8

typedef struct {
    uint8_t pin;
    const char *name;
    int fd;
} button_t;

typedef struct {
    button_t buttons[MAX_BUTTONS];
    struct pollfd pfds[MAX_BUTTONS];
    size_t count;
} button_ctx_t;

typedef void (*button_callback_t)(const button_t *button, void *user_data);

int button_ctx_init(button_ctx_t *ctx);
int button_add(button_ctx_t *ctx, uint8_t pin, const char *name);
int button_poll(button_ctx_t *ctx, int timeout_ms);
int button_handle_events(button_ctx_t *ctx, button_callback_t callback, void *user_data);
void button_cleanup(button_ctx_t *ctx);

#endif /* BUTTON_H */