#ifndef _CPU_TEMPERATURE_H__
#define _CPU_TEMPERATURE_H__

void cpu_temperature_init(void);
void cpu_temperature_deinit(void);
int cpu_temperature_get(void);

#endif // _CPU_TEMPERATURE_H__