/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"
#include <cstring>

#define BUFFSIZE    256
#define SAMPLE_RATE 48000
#define SYSTEM_CLK  270000000

int ds_factor = 1;

void downsampleAndUpsample(int32_t* signal, int factor, int length) {
    int new_size = length / factor;
    int32_t* downsampled_signal = new int32_t[new_size];

    // Downsample the signal
    for (int i = 0; i < new_size; i++) {
        downsampled_signal[i] = signal[i * factor];
    }

    // Upsample the signal
    for (int i = 0; i < length; i++) {
        signal[i] = downsampled_signal[i / factor];
    }

    // Free the memory used by the temporary arrays
    delete[] downsampled_signal;
}

void interrupt_service_routine() {
    juggle_buffers();

    int32_t *buff = mutable_data();
    int32_t left[BUFFSIZE / 2];
    int32_t right[BUFFSIZE / 2];

    for (int i = 0; i < BUFFSIZE; i += 2) {
        left[i / 2] = buff[i];
        right[i / 2] = buff[i + 1];
    }

    downsampleAndUpsample(left, ds_factor, BUFFSIZE / 2);
    downsampleAndUpsample(right, ds_factor, BUFFSIZE / 2);

    for (int i = 0; i < BUFFSIZE; i += 2) {
        buff[i] = left[i / 2];
        buff[i + 1] = right[i / 2];
    }
}

int main() {
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    tone_ale_pins_setup();
    tone_ale_capsense_setup();
    tone_ale_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    tone_ale_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);
    Capsense capsense;
    capsense.reset();

    while (true) {
        sleep_ms(1);
        ds_factor = 1 + (int)(capsense.capsense_return_percentage_of_max() * 32);
    }
}