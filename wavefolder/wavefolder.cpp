/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "common.h"

#define BUFFSIZE    16
#define SAMPLE_RATE 48000
#define GAIN        40.0
#define CLIP_POS    536870912
#define CLIP_NEG    -536870912
#define SYSTEM_CLK  270000000

constexpr float double_clip_pos = CLIP_POS*2;
constexpr float double_clip_neg = CLIP_NEG*2;

static bool state = false;

float fold(float x) {
    if(x>CLIP_POS){
        return fold(double_clip_pos - x);
    }
    else if(x<CLIP_NEG){
        return fold(double_clip_neg - x);
    }
    else return x;
}

void interrupt_service_routine() {
    juggle_buffers();

    int32_t *buff = mutable_data();
    static float y = 0;

    for(int i=0; i<BUFFSIZE; i++) {
        if(state){
            y = fold((buff[i]<<8)*GAIN);
            buff[i] = static_cast<int32_t>(y);
        }
        else {
            buff[i] = buff[i]<<8;
        }
    }
}

int main() {
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    common_pins_setup();
    common_capsense_setup();
    common_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    common_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);

    while (true) {
        sleep_ms(1);
        state = capsense_button(20);
        set_led(state);
    }
}
