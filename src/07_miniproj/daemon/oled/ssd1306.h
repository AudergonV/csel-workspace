/**
 * @file ssd1306.h
 * @brief SSD1306 OLED display driver
 * 
 * This header provides low-level I2C communication and display control
 * functions for SSD1306-based 128x64 OLED displays.
 */

#pragma once
#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

/**
 * @brief Initialize the SSD1306 OLED display
 * @return 0 on success, -1 on error
 */
int  ssd1306_init();

/**
 * @brief Set cursor position on the OLED display
 * @param column Column position (0-15 for 128 pixel width)
 * @param row Row position (0-7 for 64 pixel height)
 */
void ssd1306_set_position (uint32_t column, uint32_t row);

/**
 * @brief Display a single character at current cursor position
 * @param c Character to display (printable ASCII 32-127)
 */
void ssd1306_putc(char c);

/**
 * @brief Display a string starting at current cursor position
 * @param str Null-terminated string to display
 */
void ssd1306_puts(const char* str);

/**
 * @brief Clear the entire display
 */
void ssd1306_clear_display();

#endif