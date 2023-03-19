/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware/pio.h"
#include "capsense.pio.h"

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

bool capsense_button(int threshold) {
    int x = sense_cap(pio0, sm);

    static bool pressing = false;
    static bool state = false;

    if(x > threshold && !pressing) {
        state = !state;
    }
    pressing = (x > threshold);

    return state;
}

float capsense_return_percentage_of_max() {
    static int max = 0;
    static int min = sense_cap(pio0, sm);
    int x = sense_cap(pio0, sm);

    if(x>max) {
        max = x;
    }
    if(x<min) {
        min = x;
    }

    return (x-min) / (float) (max-min);
}
