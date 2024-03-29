/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware/pio.h"
#include "capsense.pio.h"
#include "tone_ale.h"

// #define CAP_DRIVE_PIN       16 //TODO Test if drive pin makes a difference
#define CAP_SEND_PIN        17
#define CAP_SENSE_PIN       18

int sm;

void tone_ale_capsense_setup() {
    sm = pio_claim_unused_sm(pio0, true);
    uint capsense_offset = pio_add_program(pio0, &capsense_program);
    capsense_program_init(pio0, sm, capsense_offset, CAP_SEND_PIN, CAP_SENSE_PIN);
    pio_sm_set_enabled(pio0, sm, true);
}

void Capsense::reset(){
    for(int i=0; i<32; i++) {
        capsense_button(1);
    }
    max = 0;
    min = __INT_MAX__;
}

bool Capsense::capsense_button(float threshold) {
    float x = capsense_return_percentage_of_max();

    if(x > threshold && !pressing) {
        state = !state;
    }
    pressing = (x > threshold);

    return state;
}

bool Capsense::capsense_momentary_button(int threshold) {
    int x = sense_cap(pio0, sm);
    return x > threshold;
}

float Capsense::capsense_return_percentage_of_max() {
    int x = sense_cap(pio0, sm);

    if(x>max) {
        max = x;
    }
    if(x<min) {
        min = x;
    }

    return (x-min) / (float) (max-min);
}
