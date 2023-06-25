/**
 * Copyright (c) 2022 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN         25
#define ADC_FMT_PIN     12
#define ADC_MD0_PIN     7
#define ADC_MD1_PIN     8
#define DAC_DEMP_PIN    19
#define DAC_FLT_PIN     20
#define DAC_FMT_PIN     22
#define DAC_XSMT_PIN    23


#define __24bitmax__    8388000/2
#define __24bitmin__    -8388000/2

void tone_ale_pins_setup() {
    gpio_init(LED_PIN);
    gpio_init(ADC_FMT_PIN);
    gpio_init(ADC_MD0_PIN);
    gpio_init(ADC_MD1_PIN);
    gpio_init(DAC_DEMP_PIN);
    gpio_init(DAC_FLT_PIN);
    gpio_init(DAC_FMT_PIN);
    gpio_init(DAC_XSMT_PIN);

    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(ADC_FMT_PIN, GPIO_OUT);
    gpio_set_dir(ADC_MD0_PIN, GPIO_OUT);
    gpio_set_dir(ADC_MD1_PIN, GPIO_OUT);
    gpio_set_dir(DAC_DEMP_PIN, GPIO_OUT);
    gpio_set_dir(DAC_FLT_PIN, GPIO_OUT);
    gpio_set_dir(DAC_FMT_PIN, GPIO_OUT);
    gpio_set_dir(DAC_XSMT_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    gpio_put(ADC_FMT_PIN, 0);
    gpio_put(ADC_MD0_PIN, 1);
    gpio_put(ADC_MD1_PIN, 1);
    gpio_put(DAC_DEMP_PIN, 0);
    gpio_put(DAC_FLT_PIN, 1);
    gpio_put(DAC_FMT_PIN, 0);
    gpio_put(DAC_XSMT_PIN, 1);
}

void set_led(bool state){
    gpio_put(LED_PIN, state);
}

bool get_led(){
    return gpio_get(LED_PIN);
}

int32_t clip_shift(int32_t x) {
    if(x>__24bitmax__) {return __24bitmax__<<8;}
    if(x<__24bitmin__) {return __24bitmin__<<8;}
    else {return x<<8;}
}