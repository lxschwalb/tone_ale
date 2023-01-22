/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
void common_pins_setup();
void common_capsense_setup();
void common_i2cv_setup(int32_t *buff, int buffsize, float samplerate);
bool new_buff();
int32_t * mutable_data();
bool capsense_button(int threshold);
void set_led(bool state);
