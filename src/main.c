/*
   Copyright 2021 FogML
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <zephyr.h>
#include <stdio.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include "fogml_config.h"

//#define DATA_LOGGER

float my_time_series[ACC_TIME_TICKS * ACC_AXIS];
int ticks_stored;

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

#define LED_NODE(n) DT_ALIAS(led##n)
#define LED_LABEL(n) DT_GPIO_LABEL(LED_NODE(n), gpios)
#define LED_PIN(n) DT_GPIO_PIN(LED_NODE(n), gpios)
#define LED_FLAGS(n) (GPIO_OUTPUT_INACTIVE | DT_GPIO_FLAGS(LED_NODE(n), gpios))
#define LED_DEV(n) DEVICE_DT_GET(DT_GPIO_CTLR(LED_NODE(n), gpios))

int64_t time_previous;
int64_t time_next;

#if !DT_HAS_COMPAT_STATUS_OKAY(adi_adxl362)
#error "No ADXL362!"
#endif

void led_init()
{
    const struct device *dev;

    dev = device_get_binding(LED_LABEL(0));
    if (dev != NULL)
    {
        gpio_pin_configure(dev, LED_PIN(0), GPIO_OUTPUT_ACTIVE | LED_FLAGS(0));
    }

    dev = device_get_binding(LED_LABEL(1));
    if (dev != NULL)
    {
        gpio_pin_configure(dev, LED_PIN(1), GPIO_OUTPUT_ACTIVE | LED_FLAGS(1));
    }

    dev = device_get_binding(LED_LABEL(2));
    if (dev != NULL)
    {
        gpio_pin_configure(dev, LED_PIN(2), GPIO_OUTPUT_ACTIVE | LED_FLAGS(2));
    }
}

void led_set_0(bool val)
{
    if (device_is_ready(LED_DEV(0)))
    {
        gpio_pin_set(LED_DEV(0), LED_PIN(0), val);
    }
}

void led_set_1(bool val)
{
    if (device_is_ready(LED_DEV(1)))
    {
        gpio_pin_set(LED_DEV(1), LED_PIN(1), val);
    }
}

void led_set_2(bool val)
{
    if (device_is_ready(LED_DEV(2)))
    {
        gpio_pin_set(LED_DEV(2), LED_PIN(2), val);
    }
}

void led_value(int v)
{
    led_set_0(v >> 0 & 1);
    led_set_1(v >> 1 & 1);
    led_set_2(v >> 2 & 1);
}

void main(void)
{
    struct sensor_value accel[3];

    const struct device *dev_acc = device_get_binding(DT_LABEL(DT_INST(0, adi_adxl362)));
    if (dev_acc == NULL)
    {
        printf("ACC device binding problem!\n");
        return;
    }

    led_init();

    time_previous = k_uptime_get();
    ticks_stored = 0;

    while (1)
    {
        time_next = time_previous + 1000 / FOGML_HZ;
        time_previous = k_uptime_get();

        if (time_next > time_previous)
        {
            k_msleep(time_next - time_previous);
        }

        if (sensor_sample_fetch(dev_acc) < 0)
        {
            printf("Sample fetch error!\n");
            return;
        }

        if (sensor_channel_get(dev_acc, SENSOR_CHAN_ACCEL_XYZ, &accel[0]) < 0)
        {
            printf("Channel get error!\n");
            return;
        }

        my_time_series[ticks_stored * ACC_AXIS + 0] = sensor_value_to_double(&accel[0]);
        my_time_series[ticks_stored * ACC_AXIS + 1] = sensor_value_to_double(&accel[1]);
        my_time_series[ticks_stored * ACC_AXIS + 2] = sensor_value_to_double(&accel[2]);

        ticks_stored++;

        if (ticks_stored == ACC_TIME_TICKS)
        {

#ifdef DATA_LOGGER
            fogml_features_logger(my_time_series);
#else
            int cl = fogml_classification(my_time_series);
            led_value(cl + 1);
#endif
            ticks_stored = 0;
        }
    }
}
