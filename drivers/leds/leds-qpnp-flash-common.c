// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 */

#include <linux/module.h>
#include "leds.h"

struct flash_data {
	struct list_head link;
	struct device *dev;
	int (*func)(struct led_trigger *trig, int options,
					int *max_current);
};

static LIST_HEAD(flash_common_data);

static struct led_classdev *trigger_to_lcdev(struct led_trigger *trig)
{
	struct led_classdev *led_cdev;

	read_lock(&trig->leddev_list_lock);
	list_for_each_entry(led_cdev, &trig->led_cdevs, trig_list) {
		if (!strcmp(led_cdev->default_trigger, trig->name)) {
			read_unlock(&trig->leddev_list_lock);
			return led_cdev;
		}
	}

	read_unlock(&trig->leddev_list_lock);
	return NULL;
}

int qpnp_flash_register_led_prepare(struct device *dev, void *data)
{
	struct flash_data *flash_data;

	flash_data = devm_kzalloc(dev, sizeof(struct flash_data),
						GFP_KERNEL);
	if (!flash_data)
		return -ENOMEM;

	flash_data->dev = dev;
	flash_data->func = data;

	list_add(&flash_data->link, &flash_common_data);

	return 0;
}
EXPORT_SYMBOL_GPL(qpnp_flash_register_led_prepare);

int qpnp_flash_led_prepare(struct led_trigger *trig, int options,
					int *max_current)
{
	struct flash_data *flash_data;
	struct led_classdev *led_cdev;
	int rc = -ENODEV;

	led_cdev = trigger_to_lcdev(trig);
	list_for_each_entry(flash_data, &flash_common_data, link) {
		if (led_cdev->dev->parent == flash_data->dev)
			rc = flash_data->func(trig, options, max_current);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(qpnp_flash_led_prepare);

MODULE_LICENSE("GPL");
