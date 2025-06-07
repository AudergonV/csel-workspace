#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "button.h"

#define K1_PIN 0
#define K2_PIN 2
#define K3_PIN 3

static volatile int running = 1;

#define SYSFS_MODE "/sys/class/csel/mode"
#define SYSFS_TEMP "/sys/class/csel/temp"
#define SYSFS_FREQ "/sys/class/csel/blink_freq"

static int read_file(const char *path, char *buf, size_t len)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    ssize_t r = read(fd, buf, len - 1);
    if (r < 0) {
        perror("read");
        close(fd);
        return -1;
    }
    buf[r] = '\0';
    close(fd);
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';
    return 0;
}

static int write_file(const char *path, const char *buf)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    if (write(fd, buf, strlen(buf)) < 0) {
        perror("write");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int get_mode(char *mode, size_t len)
{
    return read_file(SYSFS_MODE, mode, len);
}

static int set_mode(const char *mode)
{
    return write_file(SYSFS_MODE, mode);
}

static int get_freq(void)
{
    char buf[16];
    if (read_file(SYSFS_FREQ, buf, sizeof(buf)) < 0)
        return -1;
    return atoi(buf);
}

static int set_freq(int freq)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", freq);
    return write_file(SYSFS_FREQ, buf);
}

static void toggle_mode(void)
{
    char mode[16];
    if (get_mode(mode, sizeof(mode)) < 0)
        return;
    if (strcmp(mode, "auto") == 0)
        set_mode("manual");
    else
        set_mode("auto");
}

static void signal_handler(int sig)
{
    switch (sig) {
        case SIGINT:
            printf("\nReceived SIGINT (Ctrl+C), shutting down...\n");
            break;
        case SIGTERM:
            printf("\nReceived SIGTERM, shutting down...\n");
            break;
        default:
            printf("\nReceived signal %d, shutting down gracefully...\n", sig);
            return;
    }
    running = 0;
}

static void button_pressed(const button_t *button, void *user_data)
{
    (void)user_data; // Unused parameter

    if (strcmp(button->name, "K1") == 0) {
        int freq = get_freq();
        if (freq > 0 && freq < 20)
            freq++;
        else if (freq >= 20)
            freq = 20;
        if (freq > 0)
            set_freq(freq);
    } else if (strcmp(button->name, "K2") == 0) {
        int freq = get_freq();
        if (freq > 1)
            freq--;
        else
            freq = 1;
        if (freq > 0)
            set_freq(freq);
    } else if (strcmp(button->name, "K3") == 0) {
        toggle_mode();
    }
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
    
    printf("Button daemon started (PID: %d)\n", getpid());
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
    
    printf("\nShutting down daemon...\n");
    button_cleanup(&button_ctx);
    
    return EXIT_SUCCESS;
}

