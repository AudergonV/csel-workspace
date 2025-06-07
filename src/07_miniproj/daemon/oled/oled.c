#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ssd1306.h"
#include "oled.h"

#define POSITION_TEMP_ROW 3
#define POSITION_FREQ_ROW 4
#define POSITION_MODE_ROW 5

static char temp_str[16] = "N/A";
static char freq_str[16] = "N/A";
static char mode_str[16] = "N/A";

static void oled_format_line(int row, const char *prefix, const char *value, char *cache)
{
    if (value != NULL && strcmp(cache, value) != 0) {
        strncpy(cache, value, 15);
        cache[15] = '\0';
        
        char display_str[16];
        snprintf(display_str, sizeof(display_str), "%s%s", prefix, value);
        
        ssd1306_set_position(0, row);
        for (int i = 0; i < 16; i++) {
            ssd1306_putc(' ');
        }
        ssd1306_set_position(0, row);
        ssd1306_puts(display_str);
    }
}

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

void oled_clear()
{
    ssd1306_clear_display();
}

void oled_set_temp(const char *temp)
{
    oled_format_line(POSITION_TEMP_ROW, "Temp: ", temp, temp_str);
}

void oled_set_freq(const char *freq)
{
    char freq_with_unit[16];
    if (freq != NULL) {
        snprintf(freq_with_unit, sizeof(freq_with_unit), "%sHz", freq);
        oled_format_line(POSITION_FREQ_ROW, "Freq: ", freq_with_unit, freq_str);
    }
}

void oled_set_mode(const char *mode)
{
    oled_format_line(POSITION_MODE_ROW, "Mode: ", mode, mode_str);
}

