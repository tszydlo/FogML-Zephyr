# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(FogML-Zephyr)

set(fogml_sources
    src/fogml_sdk/anomaly/fogml_anomaly.c
    src/fogml_sdk/anomaly_rt/fogml_lof.c
    src/fogml_sdk/anomaly_rt/fogml_reservoir.c
    src/fogml_sdk/ports/fogml_ports_zephyr.c
    src/fogml_sdk/dsp/fogml_dsp.c
    src/fogml_sdk/scaler/fogml_scaler.c
)

file(GLOB fogml_generated CONFIGURE_DEPENDS
     "src/fogml_generated/*.h"
     "src/fogml_generated/*.c"
)

set(private_sources
    src/main.c)

target_sources(app PRIVATE ${private_sources} ${fogml_sources} ${fogml_generated})
