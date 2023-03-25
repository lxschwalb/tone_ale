/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"

#define BUFFSIZE        16
#define SAMPLE_RATE     48000
#define SYSTEM_CLK      270000000
#define DEPTH           154
#define SPEED_Hz        4.4

constexpr int speed = SAMPLE_RATE/SIN_BUFFSIZE/SPEED_Hz;

static int warp = 0;
static int32_t leftbuff[DEPTH] = {0};
static int leftindex = 0;
static int32_t rightbuff[DEPTH] = {0};
static int rightindex = 0;
static bool state = false;


void update_warp() {
    static int subdiv = 0;

    subdiv++;

    if(subdiv > speed) {
        warp++;
        subdiv = 0;
    }

    if(warp >= SIN_BUFFSIZE) {
        warp = 0;
    }
}

int32_t vib(int32_t x, int32_t* delaybuff, int* index) {

    delaybuff[*index] = x;
    *index = (*index+1)%DEPTH;

    return delaybuff[(*index + (int)(sin[warp]*DEPTH/2 + DEPTH/2))%DEPTH];
}

void interrupt_service_routine() {
    juggle_buffers();
    int32_t *buff = mutable_data();

    if(state){
        for (int i = 0; i < BUFFSIZE; i += 2) {
            update_warp();
            buff[i] = conv_32bit_to_24_bit(vib(correct_sign(buff[i]), leftbuff, &leftindex));
            buff[i+1] = conv_32bit_to_24_bit(vib(correct_sign(buff[i+1]), rightbuff, &rightindex));
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
        state = capsense_button(20);
        set_led(state);
    }
}
