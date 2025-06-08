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
 * Project: HEIA-FR / HES-SO MSE - MA-CSEL1 Mini Project
 *
 * Abstract: FanMgr - Fan management daemon
 * Daemon that monitors button inputs and controls system parameters 
 * through sysfs interface. Provides OLED display feedback
 * showing temperature, frequency, and operating mode status.
 *
 * Author:  Bastien Veuthey
 * Date:    07.06.2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <syslog.h>
#include "gpio/button.h"
#include "gpio/led.h"
#include "oled/oled.h"

/* GPIO pin assignments for buttons */
#define K1_PIN 0  /* Button K1: Increase frequency */
#define K2_PIN 2  /* Button K2: Decrease frequency */
#define K3_PIN 3  /* Button K3: Toggle mode */

/* GPIO pin for Power LED */
#define POWER_LED_PIN 362  /* GPIOL10/PWR-LED pin */

/* Global daemon state */
static volatile int running = 1;  /* Main loop control flag */

/* Sysfs interface paths for fan control */
#define SYSFS_MODE "/sys/devices/platform/csel/mode"       /* auto/manual mode */
#define SYSFS_TEMP "/sys/devices/platform/csel/temp"       /* temperature reading */
#define SYSFS_FREQ "/sys/devices/platform/csel/blink_freq" /* PWM/LED frequency */

/* FIFO interface for CLI commands */
#define FIFO_PATH "/tmp/fanmgr_cmd"

/* Global file descriptors */
static int fifo_fd = -1;
static int main_epoll_fd = -1;

/**
 * Read a value from a sysfs file
 * @param path Path to the sysfs file
 * @param buf Buffer to store the read value
 * @param len Buffer size
 * @return 0 on success, -1 on error
 */
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

/**
 * Write a value to a sysfs file
 * @param path Path to the sysfs file
 * @param buf Value to write
 * @return 0 on success, -1 on error
 */
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

/**
 * Get current operating mode from sysfs
 * @param mode Buffer to store mode string ("auto" or "manual")
 * @param len Buffer size
 * @return 0 on success, -1 on error
 */
static int get_mode(char *mode, size_t len)
{
    return read_file(SYSFS_MODE, mode, len);
}

/**
 * Set operating mode via sysfs
 * @param mode Mode to set ("auto" or "manual")
 * @return 0 on success, -1 on error
 */
static int set_mode(const char *mode)
{
    return write_file(SYSFS_MODE, mode);
}

/**
 * Get current PWM/LED frequency from sysfs
 * @return Frequency value on success, -1 on error
 */
static int get_freq(void)
{
    char buf[16];
    if (read_file(SYSFS_FREQ, buf, sizeof(buf)) < 0)
        return -1;
    return atoi(buf);
}

/**
 * Set PWM/LED frequency via sysfs
 * @param freq Frequency value to set (1-20 Hz)
 * @return 0 on success, -1 on error
 */
static int set_freq(int freq)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", freq);
    return write_file(SYSFS_FREQ, buf);
}

/**
 * Get current temperature reading from sysfs
 * @param temp Buffer to store temperature string
 * @param len Buffer size
 * @return 0 on success, -1 on error
 */
static int get_temp(char *temp, size_t len)
{
    return read_file(SYSFS_TEMP, temp, len);
}

/**
 * Toggle between automatic and manual operating modes
 * Switches from "auto" to "manual" or vice versa
 */
static void toggle_mode(void)
{
    char mode[16];
    int ret;
    if (get_mode(mode, sizeof(mode)) < 0) {
        syslog(LOG_ERR, "Failed to get mode");
        return;
    }
    if (strcmp(mode, "auto") == 0)
        ret = set_mode("manual");
    else
        ret = set_mode("auto");

    if (ret < 0) {
        syslog(LOG_ERR, "Failed to set mode to %s", (strcmp(mode, "auto") == 0) ? "manual" : "auto");
    } else {
        syslog(LOG_INFO, "Mode toggled to %s", (strcmp(mode, "auto") == 0) ? "manual" : "auto");
    }
}

/**
 * Update OLED display with current system metrics
 * Reads temperature, frequency, and mode from sysfs and updates display
 */
static void update_oled_display(void)
{
    char mode[16], temp[16], freq_str[16];
    int freq;
    
    if (get_mode(mode, sizeof(mode)) == 0) {
        oled_set_mode(mode);
    }
    
    if (get_temp(temp, sizeof(temp)) == 0) {
        oled_set_temp(temp);
    }
    
    freq = get_freq();
    if (freq > 0) {
        snprintf(freq_str, sizeof(freq_str), "%d", freq);
        oled_set_freq(freq_str);
    }
}

/**
 * Create and initialize FIFO for CLI communication
 * @return 0 on success, -1 on error
 */
static int init_fifo(void)
{
    unlink(FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) < 0) {
        perror("mkfifo");
        return -1;
    }
    
    fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0) {
        perror("open fifo");
        unlink(FIFO_PATH);
        return -1;
    }
    
    return 0;
}

/**
 * Process commands received from FIFO
 * @param cmd Command string to process
 */
static void process_fifo_command(const char *cmd)
{
    char *trimmed = strdup(cmd);
    char *newline = strchr(trimmed, '\n');
    if (newline) *newline = '\0';
    
    syslog(LOG_INFO, "Received command: %s", trimmed);
    
    if (strcmp(trimmed, "freq_up") == 0) {
        int freq = get_freq();
        if (freq > 0 && freq < 20) {
            if (set_freq(freq + 1) == 0) {
                syslog(LOG_INFO, "Frequency increased to %d", freq + 1);
            }
        }
    } else if (strcmp(trimmed, "freq_down") == 0) {
        int freq = get_freq();
        if (freq > 1) {
            if (set_freq(freq - 1) == 0) {
                syslog(LOG_INFO, "Frequency decreased to %d", freq - 1);
            }
        }
    } else if (strcmp(trimmed, "toggle_mode") == 0) {
        toggle_mode();
    } else {
        syslog(LOG_WARNING, "Unknown command: %s", trimmed);
    }
    
    free(trimmed);
    update_oled_display();
}

/**
 * Signal handler for graceful daemon shutdown
 * @param sig Signal number received
 */
static void signal_handler(int sig)
{
    syslog(LOG_INFO, "Received signal %d, shutting down gracefully...", sig);
    running = 0;
}

/**
 * Button press event handler
 * K1: Increase frequency (1-20 Hz)
 * K2: Decrease frequency (1-20 Hz)
 * K3: Toggle operating mode (auto/manual)
 * @param button Button structure containing button information
 * @param user_data User data (unused)
 */
static void button_pressed(const button_t *button, void *user_data)
{
    (void)user_data;
    int ret;

    // Signal button press with LED
    led_button_pressed();

    switch (button->id) {
        case BUTTON_K1: {
            syslog(LOG_DEBUG, "Button K1 pressed");
            int freq = get_freq();
            if (freq > 0 && freq < 20) {
                freq++;
            } else if (freq >= 20) {
                freq = 20;
            } else {
                syslog(LOG_ERR, "Failed to get frequency");
                return;
            }
            if (freq > 0)
                ret = set_freq(freq);
            if (ret < 0) {
                syslog(LOG_ERR, "Failed to set frequency to %d", freq);
            } else {
                syslog(LOG_DEBUG, "Frequency set to %d", freq);
            }
            break;
        }
        case BUTTON_K2: {
            syslog(LOG_DEBUG, "Button K2 pressed");
            int freq = get_freq();
            if (freq > 1) {
                freq--;
            } else if (freq == 1) {
                syslog(LOG_DEBUG, "Frequency is already at minimum");
            } else {
                syslog(LOG_ERR, "Failed to get frequency");
                return;
            }
            if (freq > 0)
                ret = set_freq(freq);
            if (ret < 0) {
                syslog(LOG_ERR, "Failed to set frequency to %d", freq);
            } else {
                syslog(LOG_DEBUG, "Frequency set to %d", freq);
            }
            break;
        }
        case BUTTON_K3:
            syslog(LOG_DEBUG, "Button K3 pressed");
            toggle_mode();
            break;
        default:
            break;
    }
}

/**
 * Button release event handler
 * Signals LED that button has been released
 * @param button Button structure containing button information
 * @param user_data User data (unused)
 */
static void button_released(const button_t *button, void *user_data)
{
    (void)user_data;
    
    syslog(LOG_DEBUG, "Button %s released", button->name);
    
    // Signal button release with LED
    led_button_released();
}

/**
 * Print program usage information
 * @param prog Program name
 */
static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -h, --help          Show this help message\n");
}

/**
 * Main daemon entry point
 * - Parses command line arguments
 * - Daemonizes the process
 * - Initializes button handling, LED, and OLED display
 * - Enters main event loop monitoring button presses
 * - Updates display periodically
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS on normal termination, EXIT_FAILURE on error
 */
int main(int argc, char *argv[])
{
    button_ctx_t button_ctx;
    struct epoll_event ev;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (daemon(0, 0) < 0) {
        syslog(LOG_ERR, "Failed to daemonize\n");
        return EXIT_FAILURE;
    }
    openlog("fanmgrd", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Fanmgr daemon started (PID: %d)", getpid());
    
    // Initialize LED
    if (led_init(POWER_LED_PIN) < 0) {
        syslog(LOG_ERR, "Failed to initialize LED");
        return EXIT_FAILURE;
    }
    
    oled_init();
    update_oled_display();
    
    signal(SIGHUP, signal_handler);  //  1 - hangup
    signal(SIGINT, signal_handler);  //  2 - terminal interrupt
    signal(SIGQUIT, signal_handler); //  3 - terminal quit
    signal(SIGABRT, signal_handler); //  6 - abort
    signal(SIGTERM, signal_handler); // 15 - termination
    signal(SIGTSTP, signal_handler); // 19 - terminal stop signal
    
    // Create main epoll instance for daemon event loop
    main_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (main_epoll_fd < 0) {
        syslog(LOG_ERR, "Failed to create main epoll instance");
        led_cleanup();
        return EXIT_FAILURE;
    }
    
    if (button_ctx_init(&button_ctx) < 0) {
        syslog(LOG_ERR, "Failed to initialize button context");
        led_cleanup();
        return EXIT_FAILURE;
    }
    
    // Set button callbacks for press and release events
    button_set_callbacks(&button_ctx, button_pressed, button_released);
    
    if (button_add(&button_ctx, K1_PIN, BUTTON_K1, "K1") < 0 ||
        button_add(&button_ctx, K2_PIN, BUTTON_K2, "K2") < 0 ||
        button_add(&button_ctx, K3_PIN, BUTTON_K3, "K3") < 0) {
        syslog(LOG_ERR, "Failed to add buttons");
        button_cleanup(&button_ctx);
        led_cleanup();
        return EXIT_FAILURE;
    }
    
    // Initialize FIFO
    if (init_fifo() < 0) {
        syslog(LOG_ERR, "Failed to initialize FIFO");
        button_cleanup(&button_ctx);
        close(main_epoll_fd);
        led_cleanup();
        return EXIT_FAILURE;
    }
    
    // Add individual button GPIO file descriptors to main epoll
    for (size_t i = 0; i < button_ctx.count; i++) {
        ev.events = EPOLLPRI | EPOLLERR;  // GPIO edge events use EPOLLPRI
        ev.data.u32 = (uint32_t)i;  // Store button index
        if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, button_ctx.buttons[i].fd, &ev) < 0) {
            syslog(LOG_ERR, "Failed to add button %zu GPIO fd to main epoll", i);
            close(fifo_fd);
            button_cleanup(&button_ctx);
            close(main_epoll_fd);
            led_cleanup();
            return EXIT_FAILURE;
        }
    }
    
    // Add FIFO to main epoll
    ev.events = EPOLLIN;
    ev.data.fd = fifo_fd;
    if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, fifo_fd, &ev) < 0) {
        syslog(LOG_ERR, "Failed to add FIFO to main epoll");
        close(fifo_fd);
        button_cleanup(&button_ctx);
        close(main_epoll_fd);
        led_cleanup();
        return EXIT_FAILURE;
    }
    
    while (running) {
        struct epoll_event main_events[10];
        int ret = epoll_wait(main_epoll_fd, main_events, 10, 1000); // 1 second timeout
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            syslog(LOG_ERR, "epoll_wait failed: %s", strerror(errno));
            break;
        }
        
        if (ret > 0) {
            for (int i = 0; i < ret; i++) {
                int fd = main_events[i].data.fd;
                
                if (fd == fifo_fd) {
                    // Handle FIFO command
                    char buf[256];
                    ssize_t n = read(fifo_fd, buf, sizeof(buf) - 1);
                    if (n > 0) {
                        buf[n] = '\0';
                        process_fifo_command(buf);
                    }
                } else {
                    // Handle GPIO button event directly
                    uint32_t button_idx = main_events[i].data.u32;
                    button_handle_event(&button_ctx, button_idx);
                }
            }
            update_oled_display();
        }
        
        static int display_update_counter = 0;
        if (++display_update_counter >= 5) { // Update display every 5 seconds
            update_oled_display();
            display_update_counter = 0;
        }
    }

    syslog(LOG_INFO, "Shutting down fanmgr daemon");
    closelog();
    
    // Cleanup resources
    if (fifo_fd >= 0) {
        close(fifo_fd);
        unlink(FIFO_PATH);
    }
    if (main_epoll_fd >= 0) {
        close(main_epoll_fd);
    }
    button_cleanup(&button_ctx);
    led_cleanup();

    return EXIT_SUCCESS;
}