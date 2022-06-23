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
float score;
int learning_samples;
bool learning = false;

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0 ""
#define PIN 0
#define FLAGS 0
#endif

int64_t time_previous;
int64_t time_next;

#if !DT_HAS_COMPAT_STATUS_OKAY(adi_adxl362)
#error "No adi,adxl362 compatible node found in the device tree"
#endif

void main(void)
{

    for(;;) {
        int num;
        num = fogml_random(0,100);
        fogml_printf_int(num);
        fogml_printf("\n");
    }

    const struct device *dev;
    // bool led_is_on = true;
    int ret;

    struct sensor_value accel[3];

    const struct device *dev_acc = device_get_binding(DT_LABEL(DT_INST(0, adi_adxl362)));

    //const struct device *dev_acc = DEVICE_DT_GET_ANY(adi_adxl372);

    if (dev_acc == NULL)
    {
        printf("Device get binding device\n");
        return;
    }

    dev = device_get_binding(LED0);
    if (dev == NULL)
    {
        printf("Device get binding device\n");
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
            printf("Sample fetch error\n");
            return;
        }

        if (sensor_channel_get(dev_acc, SENSOR_CHAN_ACCEL_XYZ, &accel[0]) < 0)
        {
            printf("Channel get error\n");
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
            if (learning)
            {
                // led_all_off();
                // digitalWrite(BLUE, LOW);
                fogml_learning(my_time_series);
                // digitalWrite(BLUE, HIGH);
                // learning_samples++;

                if (learning_samples == LEARNING_SAMPLES)
                {
                    learning_samples = 0;
                    learning = false;
                    // digitalWrite(BLUE, HIGH);
                }
            }
            else
            {
                //fogml_processing(my_time_series, &score);

                fogml_classification(my_time_series);

                if (score > 2.5)
                {
                    // digitalWrite(RED, LOW);
                }
                else
                {
                    // digitalWrite(RED, HIGH);
                }
            }

            // proximity_detection();
#endif

            ticks_stored = 0;
        }
    }

    // gpio_pin_set(dev, PIN, (int)led_is_on);
    // led_is_on = !led_is_on;
    // k_msleep(SLEEP_TIME_MS);
    // printk("***\n");
}