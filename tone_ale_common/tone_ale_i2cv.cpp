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

static int dma_out_chan = dma_claim_unused_channel(true);
static int dma_in_chan = dma_claim_unused_channel(true);

void juggle_buffers() {
    dma_hw->ints0 = 1u << dma_in_chan;

	i2cv_temp_ptr = i2cv_tx_buff_ptr;
	i2cv_tx_buff_ptr = i2cv_x_buff_ptr;
	i2cv_x_buff_ptr = i2cv_rx_buff_ptr;
	i2cv_rx_buff_ptr = i2cv_temp_ptr;
    
    dma_channel_set_write_addr(dma_in_chan, i2cv_rx_buff_ptr, true);
    dma_channel_set_read_addr(dma_out_chan, i2cv_tx_buff_ptr, true);
}

void tone_ale_clk_setup(float samplerate, float system_clk) { //  TODO use ADC to generate clock
    float freq_div = system_clk/(4*24*samplerate);
    uint i2cv_clk_offset = pio_add_program(pio0, &i2cv_clocks_program);

    // Set up state machine that drives CLK and WS pins with appropriate frequencies
    uint i2cv_clk_sm = pio_claim_unused_sm(pio0, true);
    i2cv_clocks_program_init(pio0, i2cv_clk_sm, i2cv_clk_offset, CLK_PIN, WS_PIN);
    pio_sm_set_clkdiv(pio0, i2cv_clk_sm, freq_div);
    i2cv_clocks_set_ws_widths(pio0, i2cv_clk_sm, 24, 24);
    pio_sm_set_enabled(pio0, i2cv_clk_sm, true);

    // PCM1808 requires external clock, reuse i2cv_clocks state machine to generate it
    uint adc_clk_sm = pio_claim_unused_sm(pio0, true);
    i2cv_clocks_program_init(pio0, adc_clk_sm, i2cv_clk_offset, ADC_CLK_PIN, 10); // Pin 10 is an unused dummy pin 
    pio_sm_set_clkdiv(pio0, adc_clk_sm, freq_div/8);
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
    dma_channel_config c = dma_channel_get_default_config(dma_out_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio0, i2cv_sm, true));

    dma_channel_config c_in = dma_channel_get_default_config(dma_in_chan);
    channel_config_set_transfer_data_size(&c_in, DMA_SIZE_32);
    channel_config_set_read_increment(&c_in, false);
    channel_config_set_write_increment(&c_in, true);
    channel_config_set_dreq(&c_in, pio_get_dreq(pio0, i2cv_sm, false));

    dma_channel_configure(
        dma_out_chan,
        &c,
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

    // // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_in_chan, true);

    // // Configure the processor to run interrupt_service_routine() when DMA IRQ 0 is asserted
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
