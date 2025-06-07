#include "button.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int button_ctx_init(button_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }
    
    memset(ctx, 0, sizeof(*ctx));
    return 0;
}

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

int button_poll(button_ctx_t *ctx, int timeout_ms)
{
    if (!ctx || ctx->count == 0) {
        return -1;
    }
    
    return poll(ctx->pfds, ctx->count, timeout_ms);
}

int button_handle_events(button_ctx_t *ctx, button_callback_t callback, void *user_data)
{
    if (!ctx || !callback) {
        return -1;
    }
    
    int events_handled = 0;
    
    for (size_t i = 0; i < ctx->count; i++) {
        if (ctx->pfds[i].revents & (POLLPRI | POLLERR)) {
            lseek(ctx->pfds[i].fd, 0, SEEK_SET);
            char val;
            read(ctx->pfds[i].fd, &val, 1);
            
            callback(&ctx->buttons[i], user_data);
            events_handled++;
        }
    }
    
    return events_handled;
}

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