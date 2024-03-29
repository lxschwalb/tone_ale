/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"
#include "filter_coefs.h"

#define BUFFSIZE        128
#define SAMPLE_RATE     48000
#define SYSTEM_CLK      270000000
#define ATTENUATION     4

bool state = true;
class FIR {
    public:
        int32_t inbuff[CAB_BUFFSIZE];
        int index = 0;

        int32_t apply(int32_t x) {
            int32_t sum = 0;

            inbuff[index] = x/ATTENUATION;
            index = (index+1)%CAB_BUFFSIZE;
            for(int i = 0; i < CAB_BUFFSIZE; i++) {
                sum += inbuff[(index+i)%CAB_BUFFSIZE] * cab[i];
            }
            return sum;
        }
};

FIR fir1;
FIR fir2;

void interrupt_service_routine() {
    juggle_buffers();
    int32_t * buff = mutable_data();

    if(state) {
        for(int i=0; i<BUFFSIZE; i+=2) {
            buff[i] = clip_shift(fir1.apply((buff[i]>>16)));
            buff[i+1] = clip_shift(fir2.apply((buff[i+1]>>16)));
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
    Capsense capsense;
    capsense.reset();

    while (true) {
        sleep_ms(10);
        state = capsense.capsense_button(0.5);
        set_led(state);
    }
}
