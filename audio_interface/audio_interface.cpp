#include "tusb.h"
#include "tone_ale.h"
#include "tone_ale_usb.h"
#include "pico/multicore.h" 

#define BUFFSIZE    16
#define SAMPLE_RATE 48000 // has to be 48k when usb audio in use
#define SYSTEM_CLK  270000000

bool state = false;

void core1_entry() {
    tusb_init();
    while (true)
    {
        while(multicore_fifo_rvalid()) {
            int32_t *sample_addr = (int32_t*)multicore_fifo_pop_blocking();

            if(state)
                usb_audio_buff_broker(sample_addr);
            else
                usb_audio_buff_broker_mute(sample_addr);
        }
        tud_task();
    }
}

void interrupt_service_routine() {
    juggle_buffers();

    int32_t *buff = mutable_data();

    for (int i = 0; i < BUFFSIZE; i++)
    {
        buff[i] <<= 8;
        multicore_fifo_push_blocking((uint32_t)&buff[i]);        
    }
}

int main()
{
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    tone_ale_pins_setup();
    tone_ale_capsense_setup();
    tone_ale_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    tone_ale_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);
    multicore_launch_core1(core1_entry);

    // Run muted for a little while while USB is being initialised
    for (int i = 0; i < 2048; i++)
    {
        sleep_ms(1);
        set_led(state);
    }
    
    // Loop forever
    while (true)
    {
        sleep_ms(1);
        state = capsense_button(20);
        set_led(state);
    }
}