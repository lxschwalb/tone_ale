/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "common.h"

#define BUFFSIZE 256

static const float clip = 1073741824/2;

int main() {
    stdio_init_all();
    set_sys_clock_khz(270000, true);
    common_pins_setup();
    common_i2cv_setup(BUFFSIZE);
    common_capsense_setup();

    float y = 0;
    bool state = false;
    float gain = 40;
    while (true) {
        sleep_us(10);
        if(new_buff())
        {
            int32_t *buff = mutable_data();

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
            sleep_us(100);
            state = capsense_button();
            set_led(state);
        }
    }
}
