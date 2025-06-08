/**
 * @file oled.h
 * @brief High-level OLED display interface for fan management system
 * 
 * This header provides formatted display functions for temperature, frequency,
 * and mode information on SSD1306-based OLED displays.
 */

#ifndef OLED_H
#define OLED_H

/**
 * @brief Initialize OLED display and show startup screen
 */
void oled_init();

/**
 * @brief Clear the entire OLED display
 */
void oled_clear();

/**
 * @brief Update temperature display
 * @param temp Temperature string to display
 */
void oled_set_temp(const char *temp);

/**
 * @brief Update frequency display
 * @param freq Frequency value string to display
 */
void oled_set_freq(const char *freq);

/**
 * @brief Update mode display
 * @param mode Mode string to display (e.g., "auto", "manual")
 */
void oled_set_mode(const char *mode);

#endif /* OLED_H */