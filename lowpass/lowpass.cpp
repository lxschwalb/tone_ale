/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"
#include <cstring>
#include "filter_coefs.h"

#define BUFFSIZE    256
#define SAMPLE_RATE 48000
#define SYSTEM_CLK  270000000

int foot = 0;

class LPF {
    public:
        int32_t x1 = 0, x2 = 0, y1 = 0, y2 = 0;

        int32_t apply(int32_t x) {
            int64_t y = ((LPF_b0[255 - foot] * x)>>8)  +
                        ((LPF_b1[255 - foot] * x1)>>8) +
                        ((LPF_b2[255 - foot] * x2)>>8) -
                        ((LPF_a1[255 - foot] * y1)>>8) -
                        ((LPF_a2[255 - foot] * y2)>>8);

            // y = clip_shift(y)>>8;
            
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            
            return y;
        }
};

LPF lpf1;
LPF lpf2;

void interrupt_service_routine() {
    juggle_buffers();
    int32_t *buff = mutable_data();

    for (int i = 0; i < BUFFSIZE; i += 2) {
        buff[i] = clip_shift(lpf1.apply((buff[i]>>8)));
        buff[i+1] = clip_shift(lpf2.apply((buff[i+1]>>8)));
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
        foot = capsense.capsense_return_percentage_of_max() * 255;
    }
}