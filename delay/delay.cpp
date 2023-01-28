/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "common.h"

#define BUFFSIZE        16
#define SAMPLE_RATE     48000
#define SYSTEM_CLK      270000000
#define DELAYBUFFSIZE   32768
#define FEEDBACK        true

static bool state = true;

int32_t add_delay(int32_t x) {
    static int32_t delaybuff[DELAYBUFFSIZE] = {0};
    static int index = 0;

#if FEEDBACK
    delaybuff[index] = (delaybuff[index]>>1)+x;
#else
    delaybuff[index] = x;
#endif

    index = (index+1)%DELAYBUFFSIZE;

    return x + delaybuff[index];
}

void interrupt_service_routine() {
    juggle_buffers();
    int32_t *buff = mutable_data();

    for(int i=0; i<BUFFSIZE; i++) {
        if(state){
            buff[i] = add_delay(buff[i]<<8);
        }
        else {
            buff[i] = buff[i]<<8;
        }
    }
}

int main() {
    int32_t data_buff[BUFFSIZE*3];
    // stdio_init_all();    //Uncomment this line for USB com
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
