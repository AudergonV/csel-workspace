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
 * Abstract: FanCtl - Fan management daemon control utility
 * Command line interface to control the fanmgr daemon via FIFO IPC.
 * Supports frequency control and mode switching commands.
 *
 * Author:  Bastien Veuthey
 * Date:    08.06.2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO_PATH "/tmp/fanmgr_cmd"

/**
 * Send a command to the fanmgr daemon via FIFO
 * @param command Command string to send
 * @return 0 on success, -1 on error
 */
static int send_command(const char *command)
{
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            fprintf(stderr, "Error: fanmgr daemon not running (FIFO not found)\n");
        } else {
            perror("open");
        }
        return -1;
    }
    
    if (write(fd, command, strlen(command)) < 0) {
        perror("write");
        close(fd);
        return -1;
    }
    
    if (write(fd, "\n", 1) < 0) {
        perror("write");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/**
 * Print usage information
 * @param prog Program name
 */
static void print_usage(const char *prog)
{
    printf("Usage: %s <command>\n", prog);
    printf("\nCommands:\n");
    printf("  freq_up      Increase fan frequency (1-20 Hz)\n");
    printf("  freq_down    Decrease fan frequency (1-20 Hz)\n");
    printf("  toggle_mode  Toggle between auto and manual mode\n");
    printf("  help         Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s freq_up\n", prog);
    printf("  %s toggle_mode\n", prog);
}

/**
 * Main CLI entry point
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0 || 
        strcmp(command, "--help") == 0) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    
    if (strcmp(command, "freq_up") == 0 || 
        strcmp(command, "freq_down") == 0 || 
        strcmp(command, "toggle_mode") == 0) {
        
        if (send_command(command) == 0) {
            printf("Command '%s' sent successfully\n", command);
            return EXIT_SUCCESS;
        } else {
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
}