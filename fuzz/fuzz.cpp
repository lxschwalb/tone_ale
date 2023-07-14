/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"

#define BUFFSIZE    16
#define SAMPLE_RATE 48000
#define GAIN        80.0
#define CLIP_POS    4194304<<8
#define CLIP_NEG    -4194304<<8
#define SYSTEM_CLK  270000000
#define GATED       true

static bool state = false;

int32_t fuzz(int32_t x) {
    float y = x*GAIN;            
    
    if(y>CLIP_POS){y = CLIP_POS;}
    else if(y<CLIP_NEG){y = CLIP_NEG;}

#if GATED
    else {y = 0.0;}
#endif

    return static_cast<int32_t>(y);
}

void interrupt_service_routine() {
    juggle_buffers();

    int32_t *buff = mutable_data();

    for(int i=0; i<BUFFSIZE; i++) {
        if(state){
            buff[i] = fuzz(buff[i]);
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
