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

#include "config.h"
#include "cpu_temperature.h" /* Include temperature sensor header */
#include "fan_control.h" /* Include fan control header */

ssize_t mode_show(struct device* dev,
                                    struct device_attribute* attr, char* buf)
{
    const enum fan_mode mode = fan_control_get_mode();
    if (mode == FAN_MODE_AUTO) {
        return sprintf(buf, "%s\n", CSEL_SYSFS_MODE_AUTO);
    } else if (mode == FAN_MODE_MANUAL) {
        return sprintf(buf, "%s\n", CSEL_SYSFS_MODE_MANUAL);
    }
    return sprintf(buf, "unknown\n");
}
ssize_t mode_store(struct device* dev,
                                     struct device_attribute* attr,
                                     const char* buf, size_t count)
{
    if (strncmp(buf, CSEL_SYSFS_MODE_AUTO, sizeof(CSEL_SYSFS_MODE_AUTO) - 1) == 0) {
        fan_control_set_mode(FAN_MODE_AUTO);
        pr_info("fan mode set to: %s\n", buf);
        return count;
    }
    else if (strncmp(buf, CSEL_SYSFS_MODE_MANUAL, sizeof(CSEL_SYSFS_MODE_MANUAL) - 1) == 0) {
        fan_control_set_mode(FAN_MODE_MANUAL);
        pr_info("fan mode set to: %s\n", buf);
        return count;
    }
    pr_err("invalid fan mode: %s\n", buf);
    return -EINVAL;
}

ssize_t temp_show(struct device* dev,
                                    struct device_attribute* attr, char* buf)
{
    int temp;
    int ret = cpu_temperature_get(&temp);
    if (ret < 0) {
        pr_err("failed to get CPU temperature: %d\n", ret);
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
    if (freq < CSEL_MIN_BLINK_FREQ_MANUAL_HZ) {
        freq = CSEL_MIN_BLINK_FREQ_MANUAL_HZ;
        pr_warn("blink frequency too low, setting to minimum: %d Hz\n", freq);
    } else if (freq > CSEL_MAX_BLINK_FREQ_MANUAL_HZ) {
        freq = CSEL_MAX_BLINK_FREQ_MANUAL_HZ;
        pr_warn("blink frequency too high, setting to maximum: %d Hz\n", freq);
    }
    fan_control_set_blink_freq(freq);
    pr_info("blink frequency set to: %d Hz\n", freq);
    return count;
}

DEVICE_ATTR_RO(temp);
DEVICE_ATTR_RW(mode);
DEVICE_ATTR_RW(blink_freq);

static void sysfs_device_release(struct device *dev)
{
    pr_info("sysfs device released\n");
}

static struct platform_device sysfs_device = {
    .name  = CSEL_MODULE_NAME,
    .id    = -1,
    .dev = {
        .init_name = CSEL_MODULE_NAME,
        .release = sysfs_device_release,
    },
};

static int __init mod_init(void)
{
    int status = fan_control_init();
    if (status < 0) {
        pr_err("failed to initialize fan control: %d\n", status);
        return status;
    }
    status = platform_device_register(&sysfs_device);
    if (status < 0) {
        pr_err("failed to register platform device: %d\n", status);
        return status;
    }
    status = device_create_file(&sysfs_device.dev, &dev_attr_temp);
    if (status == 0) status = device_create_file(&sysfs_device.dev, &dev_attr_mode);
    if (status == 0) status = device_create_file(&sysfs_device.dev, &dev_attr_blink_freq);
    if (status < 0) {
        pr_err("failed to create sysfs files: %d\n", status);
        platform_device_unregister(&sysfs_device);
        fan_control_deinit();
        return status;
    }
    pr_info("module loaded\n");
    return 0;
}

static void __exit mod_exit(void)
{
    device_remove_file(&sysfs_device.dev, &dev_attr_blink_freq);
    device_remove_file(&sysfs_device.dev, &dev_attr_mode);
    device_remove_file(&sysfs_device.dev, &dev_attr_temp);
    platform_device_unregister(&sysfs_device);
    fan_control_deinit();
    pr_info("module unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("Vincent Audergon <vincent.audergon@master.hes-so.ch>");
MODULE_DESCRIPTION("CSEL module");
MODULE_LICENSE("GPL");
