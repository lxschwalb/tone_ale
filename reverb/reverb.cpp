/**
 * Copyright (c) 2026 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Integer port of Freeverb (Jezar at Dreampoint, public domain):
 * 8 parallel lowpass-feedback combs into 4 series allpasses per channel,
 * with the right channel's delay lines 25 samples longer for stereo spread.
 *
 * All hot-loop arithmetic is 32-bit with Q8 coefficients so it runs fast on
 * the M0+ single-cycle multiplier (no soft-float, no 64-bit multiplies except
 * in the final wet mix, twice per frame).
 */
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "tone_ale.h"

#define BUFFSIZE    64
#define SAMPLE_RATE 48000
#define SYSTEM_CLK  270000000

// Q8 fixed-point parameters (256 = 1.0)
//
// FEEDBACK sets the room size / decay time. Freeverb maps room 0..1 to
// feedback 0.7..0.98; keep FEEDBACK <= 240 (0.9375) here - beyond that the
// comb energy at full-scale input can exceed the 32-bit headroom worked out
// below, as well as ringing basically forever.
#define FEEDBACK    215     // 0.84 = Freeverb's default room size of 0.5
#define DAMP1       51      // 0.2 = Freeverb's default damp of 0.5 x scaledamp 0.4
#define DAMP2       (256 - DAMP1)
#define INPUT_GAIN  4       // 0.0156, Freeverb uses 0.015 to keep the combs out of clipping
#define WET         85      // 0.33 wet level; dry is passed through at unity

// Freeverb's tunings are for 44.1kHz; these are the same delays rescaled to 48kHz.
#define NCOMBS          8
#define NALLPASS        4
#define STEREO_SPREAD   25
static const int comb_tuning[NCOMBS] = {1215, 1293, 1390, 1476, 1548, 1623, 1695, 1760}; // sums to 12000
static const int allpass_tuning[NALLPASS] = {605, 480, 371, 245};                        // sums to 1701
#define POOL_SIZE   (2*(12000 + 1701) + (NCOMBS + NALLPASS)*STEREO_SPREAD)               // 27702 words, ~108kB

struct Comb {
    int32_t *buf;
    int32_t store; // one-pole lowpass state in the feedback path
    int len;
    int idx;
};

struct Allpass {
    int32_t *buf;
    int len;
    int idx;
};

static int32_t pool[POOL_SIZE];
static Comb combs[2][NCOMBS];
static Allpass allpasses[2][NALLPASS];

static volatile bool state = false;

static void reverb_init() {
    int32_t *p = pool;
    for (int ch = 0; ch < 2; ch++) {
        for (int c = 0; c < NCOMBS; c++) {
            combs[ch][c].buf = p;
            combs[ch][c].len = comb_tuning[c] + ch*STEREO_SPREAD;
            combs[ch][c].idx = 0;
            combs[ch][c].store = 0;
            p += combs[ch][c].len;
        }
        for (int a = 0; a < NALLPASS; a++) {
            allpasses[ch][a].buf = p;
            allpasses[ch][a].len = allpass_tuning[a] + ch*STEREO_SPREAD;
            allpasses[ch][a].idx = 0;
            p += allpasses[ch][a].len;
        }
    }
}

// Headroom: |in| <= 2^18 (24-bit L+R sum x INPUT_GAIN), so each comb settles
// below in/(1-FEEDBACK/256) <= ~4.4M (23 bits) and a Q8 product stays inside
// int32. The damp line is a convex combination (DAMP1+DAMP2 == 256), so its
// two products together are also bounded by ~4.4M x 256.
static int32_t reverb_channel(int ch, int32_t in) {
    int32_t acc = 0;

    for (int c = 0; c < NCOMBS; c++) {
        Comb &cb = combs[ch][c];
        int32_t y = cb.buf[cb.idx];
        acc += y;
        cb.store = (y*DAMP2 + cb.store*DAMP1) >> 8;
        cb.buf[cb.idx] = in + ((cb.store*FEEDBACK) >> 8);
        if (++cb.idx >= cb.len) cb.idx = 0;
    }

    for (int a = 0; a < NALLPASS; a++) {
        Allpass &ap = allpasses[ch][a];
        int32_t y = ap.buf[ap.idx];
        int32_t out = y - acc;
        ap.buf[ap.idx] = acc + (y >> 1); // allpass feedback of 0.5 as a shift
        if (++ap.idx >= ap.len) ap.idx = 0;
        acc = out;
    }

    return acc;
}

void interrupt_service_routine() {
    juggle_buffers();
    int32_t *buff = mutable_data();

    for (int i = 0; i < BUFFSIZE; i += 2) {
        int32_t xl = buff[i] >> 8;
        int32_t xr = buff[i+1] >> 8;

        // Both channels are fed the same attenuated mono sum, as in Freeverb;
        // the stereo image comes from the spread in the delay lengths
        int32_t in = ((xl + xr) * INPUT_GAIN) >> 8;

        // Run the reverb even when bypassed so the tail decays naturally and
        // is already ringing when the effect is switched back in ("trails")
        int32_t wl = reverb_channel(0, in);
        int32_t wr = reverb_channel(1, in);

        if (state) {
            buff[i]   = clip_shift(xl + (int32_t)(((int64_t)wl * WET) >> 8));
            buff[i+1] = clip_shift(xr + (int32_t)(((int64_t)wr * WET) >> 8));
        }
    }
}

int main() {
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    tone_ale_pins_setup();
    tone_ale_capsense_setup();
    tone_ale_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    reverb_init();
    tone_ale_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);
    Capsense capsense;
    capsense.reset();

    while (true) {
        sleep_ms(10);
        state = capsense.capsense_button(0.5);
        set_led(state);
    }
}
