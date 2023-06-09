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

class Wah {
    public:
        int32_t x1 = 0, x2 = 0, y1 = 0, y2 = 0;

        int32_t apply(int32_t x) {
            int32_t y = ((BPF_b0[foot] * x)>>8 )+
                        ((BPF_b1[foot] * x1)>>8) +
                        ((BPF_b2[foot] * x2)>>8) -
                        ((BPF_a1[foot] * y1)>>8) -
                        ((BPF_a2[foot] * y2)>>8);

            y = conv_32bit_to_24_bit(y)>>8;
            
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            
            return y;
        }
};

Wah wah1;
Wah wah2;

void interrupt_service_routine() {
    juggle_buffers();
    int32_t *buff = mutable_data();

    if(foot >0){
        for (int i = 0; i < BUFFSIZE; i += 8) {
            // Downsample
            int32_t left = correct_sign(buff[i]) + correct_sign(buff[i+2]) + correct_sign(buff[i+4]) + correct_sign(buff[i+6]);
            int32_t right = correct_sign(buff[i+1]) + correct_sign(buff[i+3]) + correct_sign(buff[i+5]) + correct_sign(buff[i+7]);

            // Wah
            left = wah1.apply(left>>2)<<8;
            right = wah2.apply(right>>2)<<8;

            // Upsample
            buff[i] = left;
            buff[i+1] = right;
            buff[i+2] = left;
            buff[i+3] = right;
            buff[i+4] = left;
            buff[i+5] = right;
            buff[i+6] = left;
            buff[i+7] = right;
        }
    }
    else {
        for(int i=0; i<BUFFSIZE; i++) {
            buff[i] = buff[i] << 8;
        }
    }
}

int main() {
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    tone_ale_pins_setup();
    tone_ale_capsense_setup();
    tone_ale_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    tone_ale_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);

    while (true) {
        sleep_ms(1);
        foot = capsense_return_percentage_of_max() * 260-4;
    }
}