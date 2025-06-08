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
 * Abstract: Kernel module - CPU Fan driver
 * - Read the CPU temperature
 * - Control the CPU fan speed based on the temperature
 * - Provide sysfs interface for configuration
 * 
 * sysfs interface:
 *  - /sys/devices/platform/csel/mode: Read-write attribute to set the fan mode ("auto", "manual")
 *  - /sys/devices/platform/csel/temp: Read-only attribute to get the CPU temperature
 *  - /sys/devices/platform/csel/blink_freq: Read-write attribute to set the blink frequency of the LED, in Hz
 *
 * Author:  Vincent Audergon
 * Date:    07.06.2025
 */

#include <linux/init.h>   /* needed for macros */
#include <linux/module.h> /* needed by all modules */
#include <linux/device.h> /* needed for sysfs handling */
#include <linux/platform_device.h> /* needed for sysfs handling */

#include "cpu_temperature.h" /* Include temperature sensor header */
#include "fan_control.h" /* Include fan control header */

#define MIN_BLINK_FREQ (1)      // Hz
#define MAX_BLINK_FREQ (20)     // Hz
#define MODE_AUTO "auto"
#define MODE_AUTO_LEN (4)
#define MODE_MANUAL "manual"
#define MODE_MANUAL_LEN (6)

ssize_t mode_show(struct device* dev,
                                    struct device_attribute* attr, char* buf)
{
    const enum fan_mode mode = fan_control_get_mode();
    if (mode == FAN_MODE_AUTO) {
        return sprintf(buf, "%s\n", MODE_AUTO);
    } else if (mode == FAN_MODE_MANUAL) {
        return sprintf(buf, "%s\n", MODE_MANUAL);
    }
    return sprintf(buf, "unknown\n");
}
ssize_t mode_store(struct device* dev,
                                     struct device_attribute* attr,
                                     const char* buf, size_t count)
{
    if (strncmp(buf, MODE_AUTO, MODE_AUTO_LEN) == 0) {
        fan_control_set_mode(FAN_MODE_AUTO);
        pr_info("Fan mode set to: %s\n", buf);
        return count;
    }
    else if (strncmp(buf, MODE_MANUAL, MODE_MANUAL_LEN) == 0) {
        fan_control_set_mode(FAN_MODE_MANUAL);
        pr_info("Fan mode set to: %s\n", buf);
        return count;
    }
    pr_err("Invalid fan mode: %s\n", buf);
    return -EINVAL;
}

ssize_t temp_show(struct device* dev,
                                    struct device_attribute* attr, char* buf)
{
    int temp;
    int ret = cpu_temperature_get(&temp);
    if (ret < 0) {
        pr_err("Failed to get CPU temperature: %d\n", ret);
        return ret;
    }
    return sprintf(buf, "%d\n", temp);
}

ssize_t blink_freq_show(struct device* dev,
                                         struct device_attribute* attr, char* buf)
{
    return sprintf(buf, "%d\n", fan_control_get_blink_freq());
}

ssize_t blink_freq_store(struct device* dev,
                                          struct device_attribute* attr,
                                          const char* buf, size_t count)
{
    int freq = simple_strtol(buf, NULL, 10);
    if (freq < MIN_BLINK_FREQ || freq > MAX_BLINK_FREQ) {
        pr_err("Invalid blink frequency: %d\n", freq);
        return -EINVAL;
    }
    fan_control_set_blink_freq(freq);
    pr_info("Blink frequency set to: %d Hz\n", freq);
    return count;
}

DEVICE_ATTR_RO(temp);
DEVICE_ATTR_RW(mode);
DEVICE_ATTR_RW(blink_freq);

static void sysfs_dev_release(struct device* dev) {}
static struct platform_device sysfs_device = {
    .name        = "csel",
    .id          = -1,
    .dev.release = sysfs_dev_release,
};

static int __init mod_init(void)
{
    int status = 0;
    if (status == 0) {
        status = platform_device_register(&sysfs_device);
        pr_info("CSEL platform device registered\n");
    }
    if (status == 0) {
        status = device_create_file(&sysfs_device.dev, &dev_attr_temp);
        pr_info("CSEL sysfs file created (temp)\n");
    }
    if (status == 0) {
        status = device_create_file(&sysfs_device.dev, &dev_attr_mode);
        pr_info("CSEL sysfs file created (mode)\n");
    }
    if (status == 0) {
        status = device_create_file(&sysfs_device.dev, &dev_attr_blink_freq);
        pr_info("CSEL sysfs file created (blink_freq)\n");
    }
    if (status < 0) {
        pr_err("Failed to register CSEL platform device or create sysfs files: %d\n", status);
        return status;
    }
    status = cpu_temperature_init();
    if (status < 0) {
        pr_err("Failed to initialize CPU temperature sensor: %d\n", status);
        return status;
    }
    status = fan_control_init();
    if (status < 0) {
        pr_err("Failed to initialize fan control: %d\n", status);
        cpu_temperature_deinit();
        return status;
    }
    pr_info("CSEL module loaded\n");
    return 0;
}

static void __exit mod_exit(void)
{
    fan_control_deinit();
    cpu_temperature_deinit();
    device_remove_file(&sysfs_device.dev, &dev_attr_temp);
    device_remove_file(&sysfs_device.dev, &dev_attr_mode);
    device_remove_file(&sysfs_device.dev, &dev_attr_blink_freq);
    platform_device_unregister(&sysfs_device);
    pr_info("CSEL module unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("Vincent Audergon <vincent.audergon@master.hes-so.ch>");
MODULE_DESCRIPTION("CSEL module");
MODULE_LICENSE("GPL");
