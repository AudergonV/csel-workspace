#ifndef OLED_H
#define OLED_H

void oled_init();
void oled_clear();
void oled_set_temp(const char *temp);
void oled_set_freq(const char *freq);
void oled_set_mode(const char *mode);

#endif /* OLED_H */