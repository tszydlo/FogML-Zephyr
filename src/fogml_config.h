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

#ifndef FOGML_CONFIG_H
#define FOGML_CONFIG_H

#include "fogml_sdk/fogml.h"

#define FOGML_VERBOSE

//LEARNING
#define LEARNING_SAMPLES  16

// DATA ACQUISITION
// accelerometer data X, Y, Z
#define ACC_TIME_TICKS  20
#define ACC_AXIS  3
#define FOGML_HZ 10


// DIGITAL SIGNAL PROCESSING
// number of features - depends on the DSP blocks
#define FOGML_VECTOR_SIZE ((TINYML_DSP_BASE_LEN + TINYML_DSP_ENERGY_LEN + TINYML_DSP_CROSSINGS_LEN) * ACC_AXIS)

//BLOCK 1 - BASE
tinyml_block_base_config_t block1_config;
tinyml_dsp_block_t block1 = {
    .type = TINYML_DSP_BASE,
    .config = &block1_config

};

//BLOCK 2 - ENERGY
tinyml_block_energy_config_t block2_config;
tinyml_dsp_block_t block2 = {
    .type = TINYML_DSP_ENERGY,
    .config = &block2_config
};

//BLOCK 3 - CROSSINGS
tinyml_block_crossings_config_t block3_config = {
    .threshold = 0.00001
};
tinyml_dsp_block_t block3 = {
    .type = TINYML_DSP_CROSSINGS,
    .config = &block3_config
};

//DSP config
tinyml_dsp_block_t *blocks_tab[] = {&block1, &block2, &block3};

tinyml_dsp_config_t my_dsp_config = {
    .time_ticks = ACC_TIME_TICKS,
    .axis_n = 3,
    .blocks_num = 3,
    .blocks = blocks_tab
};

// RESERVOIR SAMPLING
#define MY_RESERVOIR_SIZE  100
float my_reservoir[MY_RESERVOIR_SIZE * FOGML_VECTOR_SIZE];

tinyml_reservoir_sampling_config_t my_rs_config = {
  .n = MY_RESERVOIR_SIZE,
  .k = 0,
  .vector_size = FOGML_VECTOR_SIZE,
  .reservoir = my_reservoir
};


// LOCAL OUTLINER FACTOR
float my_kdistance[MY_RESERVOIR_SIZE];
float my_lrd[MY_RESERVOIR_SIZE];

tinyml_lof_config_t my_lof_config = {
  .parameter_k = 3,  //k nearest neighbours are considered
  .k_distance = my_kdistance,  //table of k-distance for each of n points
  .lrd = my_lrd, //Local Reachability Density for each of n points
  .n = MY_RESERVOIR_SIZE, //number of points in data
  .vector_size = FOGML_VECTOR_SIZE, //dimension of each points
  .data = my_reservoir //set of points
};

//PIPELINES
void fogml_learning(float *time_series_data) {
#ifdef FOGML_VERBOSE
    fogml_printf("Updating reservoir.");
#endif

    float *vector = (float*)malloc(sizeof(float) * FOGML_VECTOR_SIZE);
    
    tinyml_dsp(time_series_data, vector, &my_dsp_config);

    tinyml_reservoir_sampling(vector, &my_rs_config);

#ifdef FOGML_VERBOSE
    //tinyml_reservoir_verbose(&my_rs_config);
    for(int i = 0; i < FOGML_VECTOR_SIZE; i++) {
        fogml_printf_float(vector[i]);
        fogml_printf(" ");
    }
    fogml_printf("\n");
#endif

    tinyml_lof_learn(&my_lof_config);
    
    free(vector);
}


void fogml_processing(float *time_series_data, float *score) {
    float *vector = (float*)malloc(sizeof(float) * FOGML_VECTOR_SIZE);
    tinyml_dsp(time_series_data, vector, &my_dsp_config);

#ifdef FOGML_VERBOSE
    for(int i = 0; i < FOGML_VECTOR_SIZE; i++) {
        fogml_printf_float(vector[i]);
        fogml_printf(" ");
    }
    fogml_printf("\n");
#endif

    *score = tinyml_lof_score(vector, &my_lof_config);

#ifdef FOGML_VERBOSE
    fogml_printf("LOF Score = ");
    fogml_printf_float(*score);
    fogml_printf("\n");
#endif

    free(vector);
} 

int fogml_classification(float *time_series_data) {
    float *vector = (float*)malloc(sizeof(float) * FOGML_VECTOR_SIZE);
    tinyml_dsp(time_series_data, vector, &my_dsp_config);
    int cl;

    cl = classifier(vector);

#ifdef FOGML_VERBOSE
    fogml_printf("Detected  class = ");
    fogml_printf_int(cl);
    fogml_printf("\n");
#endif

    free(vector);

    return cl;
} 


extern fogml_anomaly_config_t kmeans_anomaly_config;
extern fogml_min_max_scaler_config_t min_max_scaler_config;

float fogml_anomaly_detector(float *time_series_data) {
    float *vector = (float*)malloc(sizeof(float) * FOGML_VECTOR_SIZE);
    tinyml_dsp(time_series_data, vector, &my_dsp_config);
    float anomaly_sc;

    min_max_scaler(vector, &min_max_scaler_config);

    anomaly_sc = anomaly_score(vector, &kmeans_anomaly_config);

#ifdef FOGML_VERBOSE
    fogml_printf("Anomaly score = ");
    fogml_printf_float(anomaly_sc);
    fogml_printf("\n");
#endif

    free(vector);

    return anomaly_sc;
} 

void fogml_features_logger(float *time_series_data) {
    float *vector = (float*)malloc(sizeof(float) * FOGML_VECTOR_SIZE);
    tinyml_dsp(time_series_data, vector, &my_dsp_config);

    for(int i = 0; i < FOGML_VECTOR_SIZE; i++) {
        fogml_printf_float(vector[i]);
        
        if (i < FOGML_VECTOR_SIZE - 1) {
            fogml_printf(" ");
        }
        
    }
    fogml_printf("\n");

    free(vector);
}

#endif