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

#define SIN_BUFFSIZE    256
constexpr int speed = SAMPLE_RATE/SIN_BUFFSIZE/SPEED_Hz;

static int warp = 0;
static int32_t leftbuff[DEPTH] = {0};
static int leftindex = 0;
static int32_t rightbuff[DEPTH] = {0};
static int rightindex = 0;
static bool state = false;

static float sin[SIN_BUFFSIZE] = {0.00000000e+00,  2.46374492e-02,  4.92599411e-02,  7.38525275e-02,
        9.84002783e-02,  1.22888291e-01,  1.47301698e-01,  1.71625679e-01,
        1.95845467e-01,  2.19946358e-01,  2.43913720e-01,  2.67733003e-01,
        2.91389747e-01,  3.14869589e-01,  3.38158275e-01,  3.61241666e-01,
        3.84105749e-01,  4.06736643e-01,  4.29120609e-01,  4.51244057e-01,
        4.73093557e-01,  4.94655843e-01,  5.15917826e-01,  5.36866598e-01,
        5.57489439e-01,  5.77773831e-01,  5.97707459e-01,  6.17278221e-01,
        6.36474236e-01,  6.55283850e-01,  6.73695644e-01,  6.91698439e-01,
        7.09281308e-01,  7.26433574e-01,  7.43144825e-01,  7.59404917e-01,
        7.75203976e-01,  7.90532412e-01,  8.05380919e-01,  8.19740483e-01,
        8.33602385e-01,  8.46958211e-01,  8.59799851e-01,  8.72119511e-01,
        8.83909710e-01,  8.95163291e-01,  9.05873422e-01,  9.16033601e-01,
        9.25637660e-01,  9.34679767e-01,  9.43154434e-01,  9.51056516e-01,
        9.58381215e-01,  9.65124085e-01,  9.71281032e-01,  9.76848318e-01,
        9.81822563e-01,  9.86200747e-01,  9.89980213e-01,  9.93158666e-01,
        9.95734176e-01,  9.97705180e-01,  9.99070481e-01,  9.99829250e-01,
        9.99981027e-01,  9.99525720e-01,  9.98463604e-01,  9.96795325e-01,
        9.94521895e-01,  9.91644696e-01,  9.88165472e-01,  9.84086337e-01,
        9.79409768e-01,  9.74138602e-01,  9.68276041e-01,  9.61825643e-01,
        9.54791325e-01,  9.47177357e-01,  9.38988361e-01,  9.30229309e-01,
        9.20905518e-01,  9.11022649e-01,  9.00586702e-01,  8.89604013e-01,
        8.78081248e-01,  8.66025404e-01,  8.53443799e-01,  8.40344072e-01,
        8.26734175e-01,  8.12622371e-01,  7.98017227e-01,  7.82927610e-01,
        7.67362681e-01,  7.51331890e-01,  7.34844967e-01,  7.17911923e-01,
        7.00543038e-01,  6.82748855e-01,  6.64540179e-01,  6.45928062e-01,
        6.26923806e-01,  6.07538946e-01,  5.87785252e-01,  5.67674716e-01,
        5.47219547e-01,  5.26432163e-01,  5.05325184e-01,  4.83911424e-01,
        4.62203884e-01,  4.40215741e-01,  4.17960345e-01,  3.95451207e-01,
        3.72701992e-01,  3.49726511e-01,  3.26538713e-01,  3.03152674e-01,
        2.79582593e-01,  2.55842778e-01,  2.31947641e-01,  2.07911691e-01,
        1.83749518e-01,  1.59475791e-01,  1.35105247e-01,  1.10652682e-01,
        8.61329395e-02,  6.15609061e-02,  3.69514994e-02,  1.23196595e-02,
       -1.23196595e-02, -3.69514994e-02, -6.15609061e-02, -8.61329395e-02,
       -1.10652682e-01, -1.35105247e-01, -1.59475791e-01, -1.83749518e-01,
       -2.07911691e-01, -2.31947641e-01, -2.55842778e-01, -2.79582593e-01,
       -3.03152674e-01, -3.26538713e-01, -3.49726511e-01, -3.72701992e-01,
       -3.95451207e-01, -4.17960345e-01, -4.40215741e-01, -4.62203884e-01,
       -4.83911424e-01, -5.05325184e-01, -5.26432163e-01, -5.47219547e-01,
       -5.67674716e-01, -5.87785252e-01, -6.07538946e-01, -6.26923806e-01,
       -6.45928062e-01, -6.64540179e-01, -6.82748855e-01, -7.00543038e-01,
       -7.17911923e-01, -7.34844967e-01, -7.51331890e-01, -7.67362681e-01,
       -7.82927610e-01, -7.98017227e-01, -8.12622371e-01, -8.26734175e-01,
       -8.40344072e-01, -8.53443799e-01, -8.66025404e-01, -8.78081248e-01,
       -8.89604013e-01, -9.00586702e-01, -9.11022649e-01, -9.20905518e-01,
       -9.30229309e-01, -9.38988361e-01, -9.47177357e-01, -9.54791325e-01,
       -9.61825643e-01, -9.68276041e-01, -9.74138602e-01, -9.79409768e-01,
       -9.84086337e-01, -9.88165472e-01, -9.91644696e-01, -9.94521895e-01,
       -9.96795325e-01, -9.98463604e-01, -9.99525720e-01, -9.99981027e-01,
       -9.99829250e-01, -9.99070481e-01, -9.97705180e-01, -9.95734176e-01,
       -9.93158666e-01, -9.89980213e-01, -9.86200747e-01, -9.81822563e-01,
       -9.76848318e-01, -9.71281032e-01, -9.65124085e-01, -9.58381215e-01,
       -9.51056516e-01, -9.43154434e-01, -9.34679767e-01, -9.25637660e-01,
       -9.16033601e-01, -9.05873422e-01, -8.95163291e-01, -8.83909710e-01,
       -8.72119511e-01, -8.59799851e-01, -8.46958211e-01, -8.33602385e-01,
       -8.19740483e-01, -8.05380919e-01, -7.90532412e-01, -7.75203976e-01,
       -7.59404917e-01, -7.43144825e-01, -7.26433574e-01, -7.09281308e-01,
       -6.91698439e-01, -6.73695644e-01, -6.55283850e-01, -6.36474236e-01,
       -6.17278221e-01, -5.97707459e-01, -5.77773831e-01, -5.57489439e-01,
       -5.36866598e-01, -5.15917826e-01, -4.94655843e-01, -4.73093557e-01,
       -4.51244057e-01, -4.29120609e-01, -4.06736643e-01, -3.84105749e-01,
       -3.61241666e-01, -3.38158275e-01, -3.14869589e-01, -2.91389747e-01,
       -2.67733003e-01, -2.43913720e-01, -2.19946358e-01, -1.95845467e-01,
       -1.71625679e-01, -1.47301698e-01, -1.22888291e-01, -9.84002783e-02,
       -7.38525275e-02, -4.92599411e-02, -2.46374492e-02, -2.44929360e-16};

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