# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

set(fogml_sources
    src/fogml_sdk/anomaly_rt/fogml_lof.c
    src/fogml_sdk/anomaly_rt/fogml_reservoir.c
    src/fogml_sdk/ports/fogml_ports_zephyr.c
    src/fogml_generated/random_forest_model.c
)

set(private_sources
    src/main.c)

target_sources(app PRIVATE ${private_sources} ${fogml_sources})
