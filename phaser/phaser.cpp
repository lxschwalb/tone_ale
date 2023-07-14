/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"

#define BUFFSIZE    16
#define SAMPLE_RATE 48000
#define SYSTEM_CLK  270000000

#define PHASEBUFFSIZE   16
#define NUM_PHASES      4
#define FB_GAIN         0.13
#define SPEED_Hz        2
#define FLAVOUR         17

constexpr int speed = SAMPLE_RATE/SIN_BUFFSIZE/SPEED_Hz;
constexpr int phaseoffset = SIN_BUFFSIZE/NUM_PHASES/FLAVOUR;

static int32_t leftbuff[PHASEBUFFSIZE] = {0};
static int leftindex = PHASEBUFFSIZE/2;
static int32_t rightbuff[PHASEBUFFSIZE] = {0};
static int rightindex = 0;
int modIndex = 0;

static bool state = false;

int32_t phaser(float input, int32_t* delaybuff, int* delayIndex)
{
    delaybuff[*delayIndex] = input;
    int32_t output = 0;
    for (int i = 0; i < NUM_PHASES; i++) {
        int phaseShift = PHASEBUFFSIZE/2 + PHASEBUFFSIZE/2 * sin[(modIndex + phaseoffset * i)%SIN_BUFFSIZE];
        output += delaybuff[(*delayIndex + phaseShift) % PHASEBUFFSIZE];
    }

    delaybuff[*delayIndex] += FB_GAIN * output;

    *delayIndex = (*delayIndex+1) % PHASEBUFFSIZE;
    
    return (output/NUM_PHASES + input)/2;
}

void modulate() {
    static int subdiv = 0;
    subdiv++;

    if(subdiv > speed) {
        modIndex++;
        subdiv = 0;
    }

    if(modIndex >= SIN_BUFFSIZE) {
        modIndex = 0;
    }
}

void interrupt_service_routine() {
    juggle_buffers();

    int32_t *buff = mutable_data();

    if(state){
        for (int i = 0; i < BUFFSIZE; i += 2) {
            modulate();
            buff[i] = clip_shift(phaser(buff[i]>>8, leftbuff, &leftindex));
            buff[i+1] = clip_shift(phaser(buff[i+1]>>8, rightbuff, &rightindex));
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
