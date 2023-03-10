/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
void common_pins_setup();
void common_capsense_setup();
void juggle_buffers();
void common_clk_setup(float sample_rate, float system_clk);
void common_i2cv_setup(int32_t *buff, int buffsize, void interrupt_service_routine());
int32_t * mutable_data();
bool capsense_button(int threshold);
float capsense_return_percentage_of_max();
void set_led(bool state);
void usb_audio_buff_broker(int32_t *buff);
void usb_audio_buff_broker_mute(int32_t *buff);
