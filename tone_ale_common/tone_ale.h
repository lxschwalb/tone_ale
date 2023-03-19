/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
void tone_ale_pins_setup();
void tone_ale_capsense_setup();
void juggle_buffers();
void tone_ale_clk_setup(float sample_rate, float system_clk);
void tone_ale_i2cv_setup(int32_t *buff, int buffsize, void interrupt_service_routine());
int32_t * mutable_data();
bool capsense_button(int threshold);
float capsense_return_percentage_of_max();
void set_led(bool state);
int32_t correct_sign(int32_t x);
int32_t conv_32bit_to_24_bit(int32_t x);
