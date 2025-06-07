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
 * Autĥor:  Vincent Audergon
 * Date:    07.06.2025
 */

#include <linux/init.h>   /* needed for macros */
#include <linux/module.h> /* needed by all modules */
#include <linux/device.h> /* needed for sysfs handling */

static int __init mod_init(void)
{
    pr_info("CSEL module loaded\n");
    return 0;
}

static void __exit mod_exit(void)
{
    pr_info("CSEL module unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("Vincent Audergon <vincent.audergon@master.hes-so.ch>");
MODULE_DESCRIPTION("CSEL module");
MODULE_LICENSE("GPL");
