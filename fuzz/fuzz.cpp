/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "common.h"

#define BUFFSIZE 16
#define SAMPLE_RATE 48000

static const float clip = 1073741824/2;

int main() {
    int32_t data_buff[BUFFSIZE*3];
    // stdio_init_all();
    set_sys_clock_khz(270000, true);
    common_pins_setup();
    common_capsense_setup();
    common_i2cv_setup(data_buff, BUFFSIZE, SAMPLE_RATE);

    float y = 0;
    bool state = false;
    float gain = 40;
    int32_t *buff = mutable_data();
    while (true) {
        // sleep_us(10);
        if(new_buff())
        {
            buff = mutable_data();

            for(int i=0; i<BUFFSIZE; i++) {
                if(state){
                    y = (buff[i]<<8)*gain;
                    
                    if(y>clip){y = clip;}
                    if(y<-clip){y = -clip;}

                    buff[i] = static_cast<int32_t>(y);
                }
                else {
                    buff[i] = buff[i]<<8;
                }
            }
        }
        else
        {
            sleep_us(10);
            state = capsense_button(20);
            set_led(state);
        }
    }
}
