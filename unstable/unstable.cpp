/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "tone_ale.h"
#include <cstring>

#define BUFFSIZE    256
#define SAMPLE_RATE 48000
#define SYSTEM_CLK  270000000

bool state = true;

class Unstable {
    public:
        float b0, b1, b2, a1, a2;
        int32_t x1 = 0, x2 = 0, y1 = 0, y2 = 0;

        void calculate_coefs() {
            float omega = 1/48;
            float alpha = sin[(int)(omega*SIN_BUFFSIZE)];
            float cosw = sin[(int)((omega+0.75)*SIN_BUFFSIZE)%SIN_BUFFSIZE];
            float a0inv = 1.0 / (1.0 + alpha);
            a1 = -2 * cosw * a0inv;
            a2 = (1.0 - alpha) * a0inv;

            b0 = 1;
            b1 = 0;
            b2 = -1;
        }

        int32_t apply(int32_t x) {
            int32_t y = (int)(b0 * x) + (int)(b1 * x1) + (int)(b2 * x2) - (int)(a1 * y1) - (int)(a2 * y2);
            
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            
            return y;
        }
};

Unstable unstable1;
Unstable unstable2;

void interrupt_service_routine() {
    juggle_buffers();
    int32_t *buff = mutable_data();

    if(state) {
        for (int i = 0; i < BUFFSIZE; i += 2) {
            buff[i] = unstable1.apply(buff[i])>>2;
            buff[i+1] = unstable2.apply(buff[i+1])>>2;
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

    unstable1.calculate_coefs();
    unstable2.calculate_coefs();
    while (true) {
        sleep_ms(20);
        state = capsense.capsense_button(0.5);
        set_led(state);
    }
}