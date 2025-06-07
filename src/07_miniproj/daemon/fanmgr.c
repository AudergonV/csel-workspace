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
#include <syslog.h>
#include "gpio/button.h"
#include "oled/oled.h"

/* GPIO pin assignments for buttons */
#define K1_PIN 0  /* Button K1: Increase frequency */
#define K2_PIN 2  /* Button K2: Decrease frequency */
#define K3_PIN 3  /* Button K3: Toggle mode */

/* Global daemon state */
static volatile int running = 1;  /* Main loop control flag */

/* Sysfs interface paths for fan control */
#define SYSFS_MODE "/sys/devices/platform/csel/mode"       /* auto/manual mode */
#define SYSFS_TEMP "/sys/devices/platform/csel/temp"       /* temperature reading */
#define SYSFS_FREQ "/sys/devices/platform/csel/blink_freq" /* PWM/LED frequency */


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

    if (strcmp(button->name, "K1") == 0) {
        syslog(LOG_INFO, "Button K1 pressed");
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
            syslog(LOG_INFO, "Frequency set to %d", freq);
        }
    } else if (strcmp(button->name, "K2") == 0) {
        syslog(LOG_INFO, "Button K2 pressed");
        int freq = get_freq();
        if (freq > 1) {
            freq--;
        } else if (freq == 1) {
            syslog(LOG_INFO, "Frequency is already at minimum");
        } else {
            syslog(LOG_ERR, "Failed to get frequency");
            return;
        }
        if (freq > 0)
            ret = set_freq(freq);
        if (ret < 0) {
            syslog(LOG_ERR, "Failed to set frequency to %d", freq);
        } else {
            syslog(LOG_INFO, "Frequency set to %d", freq);
        }
    } else if (strcmp(button->name, "K3") == 0) {
        syslog(LOG_INFO, "Button K3 pressed");
        toggle_mode();
    }
}

/**
 * Daemonize the process using double-fork technique
 * - Forks twice to ensure process becomes a proper daemon
 * - Detaches from controlling terminal
 * - Changes working directory to root
 * - Redirects standard streams to /dev/null
 * @return 0 on success, -1 on error
 */
static int daemonize(void)
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid");
        return -1;
    }

    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) {
        perror("second fork");
        return -1;
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (chdir("/") < 0) {
        perror("chdir");
        return -1;
    }

    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);

    return 0;
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
 * - Initializes button handling and OLED display
 * - Enters main event loop monitoring button presses
 * - Updates display periodically
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS on normal termination, EXIT_FAILURE on error
 */
int main(int argc, char *argv[])
{
    button_ctx_t button_ctx;
    
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
    
    if (daemonize() < 0) {
        syslog(LOG_ERR, "Failed to daemonize\n");
        return EXIT_FAILURE;
    }
    openlog("fanmgrd", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Fanmgr daemon started (PID: %d)", getpid());
    
    oled_init();
    update_oled_display();
    
    signal(SIGHUP, signal_handler);  //  1 - hangup
    signal(SIGINT, signal_handler);  //  2 - terminal interrupt
    signal(SIGQUIT, signal_handler); //  3 - terminal quit
    signal(SIGABRT, signal_handler); //  6 - abort
    signal(SIGTERM, signal_handler); // 15 - termination
    signal(SIGTSTP, signal_handler); // 19 - terminal stop signal
    
    if (button_ctx_init(&button_ctx) < 0) {
        syslog(LOG_ERR, "Failed to initialize button context");
        return EXIT_FAILURE;
    }
    
    if (button_add(&button_ctx, K1_PIN, "K1") < 0 ||
        button_add(&button_ctx, K2_PIN, "K2") < 0 ||
        button_add(&button_ctx, K3_PIN, "K3") < 0) {
        syslog(LOG_ERR, "Failed to add buttons");
        button_cleanup(&button_ctx);
        return EXIT_FAILURE;
    }
    
    while (running) {
        int ret = button_poll(&button_ctx, 1000);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            syslog(LOG_ERR, "poll failed: %s", strerror(errno));
            break;
        }
        
        if (ret > 0) {
            button_handle_events(&button_ctx, button_pressed, NULL);
            update_oled_display();
        }
        
        static int display_update_counter = 0;
        if (++display_update_counter >= 5) {
            update_oled_display();
            display_update_counter = 0;
        }
    }

    syslog(LOG_INFO, "Shutting down fanmgr daemon");
    closelog();
    button_cleanup(&button_ctx);
    
    return EXIT_SUCCESS;
}

