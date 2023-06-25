/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "i2cv.pio.h"

#define ADC_CLK_PIN         11
#define CLK_PIN             14
#define WS_PIN              15
#define I2CV_OUT_PIN        21
#define I2CV_IN_PIN         13

static int32_t *i2cv_tx_buff_ptr;
static int32_t *i2cv_rx_buff_ptr;
static int32_t *i2cv_x_buff_ptr;
static int32_t *i2cv_temp_ptr;

static int32_t **i2cv_tx_buff_ptr_ptr = &i2cv_x_buff_ptr;
static int32_t **i2cv_rx_buff_ptr_ptr = &i2cv_tx_buff_ptr;

static int dma_out_chan = dma_claim_unused_channel(true);
static int dma_in_chan = dma_claim_unused_channel(true);
static int dma_out_conf_chan = dma_claim_unused_channel(true);
static int dma_in_conf_chan = dma_claim_unused_channel(true);

void juggle_buffers() {
    dma_hw->ints0 = 1u << dma_in_chan;

	i2cv_temp_ptr = i2cv_tx_buff_ptr;
	i2cv_tx_buff_ptr = i2cv_x_buff_ptr;
	i2cv_x_buff_ptr = i2cv_rx_buff_ptr;
	i2cv_rx_buff_ptr = i2cv_temp_ptr;
}

void tone_ale_clk_setup(float samplerate, float system_clk) {
    // We generate a system clock for the ADC, then the ADC generates the WS and bitclock signals 
    float freq_div = system_clk/(2*256*samplerate);
    uint simple_clock_offset = pio_add_program(pio0, &simple_clock_program);
    uint adc_clk_sm = pio_claim_unused_sm(pio0, true);
    simple_clock_program_init(pio0, adc_clk_sm, simple_clock_offset, ADC_CLK_PIN);
    pio_sm_set_clkdiv(pio0, adc_clk_sm, freq_div);
    pio_sm_set_enabled(pio0, adc_clk_sm, true);
}

void tone_ale_i2cv_setup(int32_t *buff, int buffsize, void interrupt_service_routine()) {
    i2cv_tx_buff_ptr = buff;
    i2cv_rx_buff_ptr = &buff[buffsize];
    i2cv_x_buff_ptr = &buff[buffsize*2];

    // Set up state machine to send I2CV data in and out
    uint i2cv_offset = pio_add_program(pio0, &i2cv_bidirectional_program);
    uint i2cv_sm = pio_claim_unused_sm(pio0, true);
    i2cv_bidirectional_program_init(pio0, i2cv_sm, i2cv_offset, I2CV_OUT_PIN, WS_PIN, I2CV_IN_PIN);
    pio_sm_set_enabled(pio0, i2cv_sm, true);

    // Set up DMA for efficient transfers of buffers
    dma_channel_config c_out = dma_channel_get_default_config(dma_out_chan);
    channel_config_set_transfer_data_size(&c_out, DMA_SIZE_32);
    channel_config_set_read_increment(&c_out, true);
    channel_config_set_write_increment(&c_out, false);
    channel_config_set_dreq(&c_out, pio_get_dreq(pio0, i2cv_sm, true));
    channel_config_set_chain_to(&c_out, dma_out_conf_chan);

    dma_channel_config c_in = dma_channel_get_default_config(dma_in_chan);
    channel_config_set_transfer_data_size(&c_in, DMA_SIZE_32);
    channel_config_set_read_increment(&c_in, false);
    channel_config_set_write_increment(&c_in, true);
    channel_config_set_dreq(&c_in, pio_get_dreq(pio0, i2cv_sm, false));
    channel_config_set_chain_to(&c_in, dma_in_conf_chan);

    dma_channel_config c_out_conf = dma_channel_get_default_config(dma_out_conf_chan);
    channel_config_set_transfer_data_size(&c_out_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&c_out_conf, false);
    channel_config_set_write_increment(&c_out_conf, false);
    channel_config_set_dreq(&c_out_conf, 0x3f);
    channel_config_set_chain_to(&c_out_conf, dma_out_chan);

    dma_channel_config c_in_conf = dma_channel_get_default_config(dma_in_conf_chan);
    channel_config_set_transfer_data_size(&c_in_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&c_in_conf, false);
    channel_config_set_write_increment(&c_in_conf, false);
    channel_config_set_dreq(&c_in_conf, 0x3f);
    channel_config_set_chain_to(&c_in_conf, dma_in_chan);

    dma_channel_configure(
        dma_out_chan,
        &c_out,
        &pio0_hw->txf[i2cv_sm],
        i2cv_tx_buff_ptr,
        buffsize,
        false
    );

    dma_channel_configure(
        dma_in_chan,
        &c_in,
        i2cv_rx_buff_ptr,
        &pio0_hw->rxf[i2cv_sm],
        buffsize,
        false
    );

    dma_channel_configure(
        dma_out_conf_chan,
        &c_out_conf,
        &dma_hw->ch[dma_out_chan].read_addr,
        i2cv_tx_buff_ptr_ptr,
        1,
        false
    );

    dma_channel_configure(
        dma_in_conf_chan,
        &c_in_conf,
        &dma_hw->ch[dma_in_chan].write_addr,
        i2cv_rx_buff_ptr_ptr,
        1,
        false
    );

    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_in_chan, true);

    // Configure the processor to run interrupt_service_routine() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, interrupt_service_routine);
    irq_set_enabled(DMA_IRQ_0, true);

    // Wait for falling edge, then trigger DMAs
    while(gpio_get(WS_PIN) == false);
    while(gpio_get(WS_PIN));
    dma_channel_set_write_addr(dma_in_chan, i2cv_rx_buff_ptr, true); // TODO use multi chan trigger
    dma_channel_set_read_addr(dma_out_chan, i2cv_tx_buff_ptr, true);
}

int32_t * mutable_data() {
    return i2cv_x_buff_ptr;
}
