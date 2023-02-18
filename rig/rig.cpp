/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "common.h"

#define BUFFSIZE        128
#define SAMPLE_RATE     48000
#define SYSTEM_CLK      270000000
#define CAB_BUFFSIZE    128
#define ATTENUATION     7
#define GAIN        80.0
#define CLIP_POS    4194304
#define CLIP_NEG    -4194304

static const int32_t cab[CAB_BUFFSIZE] = {  -1,
                                            -2,
                                            -2,
                                            -3,
                                            -3,
                                            -3,
                                            -4,
                                            -4,
                                            -4,
                                            -4,
                                            -5,
                                            -5,
                                            -5,
                                            -5,
                                            -5,
                                            -6,
                                            -6,
                                            -6,
                                            -6,
                                            -6,
                                            -7,
                                            -7,
                                            -7,
                                            -7,
                                            -7,
                                            -8,
                                            -8,
                                            -8,
                                            -8,
                                            -8,
                                            -9,
                                            -9,
                                            -9,
                                            -9,
                                            -9,
                                            -10,
                                            -10,
                                            -10,
                                            -10,
                                            -10,
                                            -10,
                                            -10,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -11,
                                            -10,
                                            -10,
                                            -10,
                                            -10,
                                            -10,
                                            -9,
                                            -10,
                                            -10,
                                            -11,
                                            -13,
                                            -15,
                                            -16,
                                            -17,
                                            -15,
                                            -10,
                                            -3,
                                            7,
                                            15,
                                            18,
                                            11,
                                            -10,
                                            -43,
                                            -83,
                                            -117,
                                            -131,
                                            -111,
                                            -52,
                                            37,
                                            136,
                                            215,
                                            245,
                                            217,
                                            148,
                                            73,
                                            22,
                                            2,
                                            -5,
                                            -10,
                                            -12,
                                            -12,
                                            -10,
                                            -7,
                                            -3};

int32_t correct_sign(int32_t x) {
    if (x & (1<<23))
    {
        return x | 0xFF000000;
    }
    else
    {
        return x;
    }
}

int32_t fuzz(int32_t x) {
    float y = x*GAIN;
            
    if(y>CLIP_POS){y = CLIP_POS;}
    if(y<CLIP_NEG){y = CLIP_NEG;}

    return static_cast<int32_t>(y);
}

int32_t fir(int32_t x, bool channel_select) {
    static int32_t inbuff1[CAB_BUFFSIZE];
    static int32_t inbuff2[CAB_BUFFSIZE];
    static int index1 = 0;
    static int index2 = 0;
    int32_t sum = 0;

    if(channel_select) {
        inbuff1[index1] = x/ATTENUATION;
        index1 = (index1+1)%CAB_BUFFSIZE;
        for(int i = 0; i < CAB_BUFFSIZE; i++) {
            sum += inbuff1[(index1+i)%CAB_BUFFSIZE] * cab[i];
        }
    }
    else {
        inbuff2[index2] = x/ATTENUATION;
        index2 = (index2+1)%CAB_BUFFSIZE;
        for(int i = 0; i < CAB_BUFFSIZE; i++) {
            sum += inbuff2[(index2+i)%CAB_BUFFSIZE] * cab[i];
        }
    }
    return sum;
}

void interrupt_service_routine() {
    juggle_buffers();
    int32_t * buff = mutable_data();

    for(int i=0; i<BUFFSIZE; i++) {
        buff[i] = fir(fuzz(correct_sign(buff[i])), i%2);
    }
}

int main() {
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    common_pins_setup();
    common_capsense_setup();
    common_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    common_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);
    set_led(true);

    while (true) {
        // state = capsense_button(20);
    }
}
