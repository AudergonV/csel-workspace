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
 * Abstract: OLED display management library
 * High-level OLED display interface for fan management system.
 * Provides formatted display functions for temperature, frequency,
 * and mode information on SSD1306-based OLED displays.
 *
 * Author:  Bastien Veuthey
 * Date:    07.06.2025
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ssd1306.h"
#include "oled.h"

/* Display layout configuration */
#define POSITION_TEMP_ROW 3  /* Row for temperature display */
#define POSITION_FREQ_ROW 4  /* Row for frequency display */
#define POSITION_MODE_ROW 5  /* Row for mode display */

/* Cached display values to avoid unnecessary updates */
static char temp_str[16] = "N/A";  /* Last displayed temperature */
static char freq_str[16] = "N/A";  /* Last displayed frequency */
static char mode_str[16] = "N/A";  /* Last displayed mode */

/**
 * Internal function to format and update a display line
 * Only updates the display if the value has changed from the cached value
 * @param row Display row number (0-7)
 * @param prefix Text prefix to display before the value
 * @param value New value to display
 * @param cache Cached value string for comparison
 */
static void oled_format_line(int row, const char *prefix, const char *value, char *cache)
{
    /* Only update if value has changed */
    if (value != NULL && strcmp(cache, value) != 0) {
        // Update cache with new value
        strncpy(cache, value, 15);
        cache[15] = '\0';
        
        // Format display string with prefix
        char display_str[16];
        snprintf(display_str, sizeof(display_str), "%s%s", prefix, value);
        
        // Clear the line and write new content
        ssd1306_set_position(0, row);
        for (int i = 0; i < 16; i++) {
            ssd1306_putc(' ');
        }
        ssd1306_set_position(0, row);
        ssd1306_puts(display_str);
    }
}

/**
 * Initialize OLED display and show startup screen
 * Sets up the display with title, headers, and initial values
 */
void oled_init()
{
    if (ssd1306_init() == 0) {
        ssd1306_set_position(0, 0);
        ssd1306_puts("CSEL1a - SP.25");
        ssd1306_set_position(0, 1);
        ssd1306_puts("  Fan Manager");
        ssd1306_set_position(0, 2);
        ssd1306_puts("--------------");

        ssd1306_set_position(0, POSITION_TEMP_ROW);
        ssd1306_puts("Temp: N/A");
        ssd1306_set_position(0, POSITION_FREQ_ROW);
        ssd1306_puts("Freq: N/A");
        ssd1306_set_position(0, POSITION_MODE_ROW);
        ssd1306_puts("Mode: N/A");
    }
}

/**
 * Clear the entire OLED display
 * Erases all content from the display
 */
void oled_clear()
{
    ssd1306_clear_display();
}

/**
 * Update temperature display
 * Shows the current temperature reading on the display
 * @param temp Temperature string to display
 */
void oled_set_temp(const char *temp)
{
    oled_format_line(POSITION_TEMP_ROW, "Temp: ", temp, temp_str);
}

/**
 * Update frequency display
 * Shows the current PWM frequency with Hz unit
 * @param freq Frequency value string to display
 */
void oled_set_freq(const char *freq)
{
    char freq_with_unit[16];
    if (freq != NULL) {
        snprintf(freq_with_unit, sizeof(freq_with_unit), "%sHz", freq);
        oled_format_line(POSITION_FREQ_ROW, "Freq: ", freq_with_unit, freq_str);
    }
}

/**
 * Update mode display
 * Shows the current operating mode (auto/manual)
 * @param mode Mode string to display
 */
void oled_set_mode(const char *mode)
{
    oled_format_line(POSITION_MODE_ROW, "Mode: ", mode, mode_str);
}

