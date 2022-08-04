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

#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0 ""
#define PIN 0
#define FLAGS 0
#endif

int64_t time_previous;
int64_t time_next;

#if !DT_HAS_COMPAT_STATUS_OKAY(adi_adxl362)
#error "No ADXL362!"
#endif

void main(void)
{
    const struct device *dev;
    int ret;

    struct sensor_value accel[3];

    const struct device *dev_acc = device_get_binding(DT_LABEL(DT_INST(0, adi_adxl362)));
    if (dev_acc == NULL)
    {
        printf("ACC device binding problem!\n");
        return;
    }

    dev = device_get_binding(LED0);
    if (dev == NULL)
    {
        printf("LED device binding problem!\n");
        return;
    }

    ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
    if (ret < 0)
    {
        return;
    }

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
            fogml_classification(my_time_series);
#endif
            ticks_stored = 0;
        }
    }
}