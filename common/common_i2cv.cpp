/**
 * Copyright (c) 2023 Tone Age Technology
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "i2cv.pio.h"

#define BUFFSIZE 256 // TODO make resizable

#define ADC_CLK_PIN         11
#define CLK_PIN             14
#define WS_PIN              15
#define I2CV_OUT_PIN        21
#define I2CV_IN_PIN         13
#define FREQ_DIV            256

static int32_t *i2cv_tx_buff_ptr;
static int32_t *i2cv_rx_buff_ptr;
static int32_t *i2cv_x_buff_ptr;
static int32_t *i2cv_temp_ptr;

static bool buff_ready;

int dma_out_chan = 0;   // TODO autodetect unused dma chan
int dma_in_chan = 1;    // TODO autodetect unused dma chan

void dma_handler() {
    dma_hw->ints0 = 1u << dma_out_chan;

	i2cv_temp_ptr = i2cv_tx_buff_ptr;
	i2cv_tx_buff_ptr = i2cv_x_buff_ptr;
	i2cv_x_buff_ptr = i2cv_rx_buff_ptr;
	i2cv_rx_buff_ptr = i2cv_temp_ptr;
    buff_ready = true;

    dma_channel_set_read_addr(dma_out_chan, i2cv_tx_buff_ptr, true);
    dma_channel_set_write_addr(dma_in_chan, i2cv_rx_buff_ptr, true);
}

void common_i2cv_setup(int buffsize) {
    static int32_t i2cv_data[BUFFSIZE * 3];
    i2cv_tx_buff_ptr = i2cv_data;
    i2cv_rx_buff_ptr = &i2cv_data[buffsize];
    i2cv_x_buff_ptr = &i2cv_data[buffsize*2];

    uint i2cv_offset = pio_add_program(pio0, &i2cv_bidirectional_program);
    uint i2cv_clk_offset = pio_add_program(pio1, &i2cv_clocks_program);

    // Set up state machine to send I2CV data in and out
    uint i2cv_sm = pio_claim_unused_sm(pio0, true);;
    i2cv_bidirectional_program_init(pio0, i2cv_sm, i2cv_offset, I2CV_OUT_PIN, WS_PIN, I2CV_IN_PIN);
    pio_sm_set_enabled(pio0, i2cv_sm, true);

    // Set up state machine that drives CLK and WS pins with appropriate frequencies
    uint i2cv_clk_sm = pio_claim_unused_sm(pio1, true);
    i2cv_clocks_program_init(pio1, i2cv_clk_sm, i2cv_clk_offset, CLK_PIN, WS_PIN);
    pio_sm_set_clkdiv(pio1, i2cv_clk_sm, FREQ_DIV);
    i2cv_clocks_set_ws_widths(pio1, i2cv_clk_sm, 24, 24);
    pio_sm_set_enabled(pio1, i2cv_clk_sm, true);

    // PCM1808 requires external clock, reuse i2cv_clocks state machine to generate it
    uint adc_clk_sm = pio_claim_unused_sm(pio1, true);
    i2cv_clocks_program_init(pio1, adc_clk_sm, i2cv_clk_offset, ADC_CLK_PIN, 10);
    pio_sm_set_clkdiv(pio1, adc_clk_sm, FREQ_DIV>>3);
    pio_sm_set_enabled(pio1, adc_clk_sm, true);

    // Set up DMA for efficient transfers of buffers
    dma_channel_config c = dma_channel_get_default_config(dma_out_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_PIO0_TX0); //TODO make so that it's not hardcoded to PIO0 sm0

    dma_channel_config c_in = dma_channel_get_default_config(dma_in_chan);
    channel_config_set_transfer_data_size(&c_in, DMA_SIZE_32);
    channel_config_set_read_increment(&c_in, false);
    channel_config_set_write_increment(&c_in, true);
    channel_config_set_dreq(&c_in, DREQ_PIO0_RX0); //TODO make so that it's not hardcoded to PIO0 sm0

    dma_channel_configure(
        dma_out_chan,
        &c,
        &pio0_hw->txf[i2cv_sm], //TODO is this correct?
        i2cv_tx_buff_ptr,
        buffsize,
        false
    );

    dma_channel_configure(
        dma_in_chan,
        &c_in,
        i2cv_rx_buff_ptr,
        &pio0_hw->rxf[i2cv_sm],  //TODO is this correct?
        buffsize,
        false
    );

    // // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_out_chan, true);

    // // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Manually call the handler once, to trigger the first transfer
    dma_handler();
}

bool new_buff() {
    if(buff_ready) {
        buff_ready = false;
        return true;
    }
    else {
        return false;
    }
}

int32_t * mutable_data() {
    return i2cv_x_buff_ptr;
}