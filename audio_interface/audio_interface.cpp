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
    int32_t *sample_addr;
    while (true)
    {
        tud_task();
        while(multicore_fifo_rvalid()) {
            sample_addr = (int32_t*)multicore_fifo_pop_blocking();

            if(state){
                for (int i = 0; i < BUFFSIZE; i++){
                    usb_audio_buff_broker(&sample_addr[i]);
                }
            }
            else
                for (int i = 0; i < BUFFSIZE; i++)
                    usb_audio_buff_broker_mute(&sample_addr[i]);
        }
    }
}

void interrupt_service_routine() {
    juggle_buffers();

    multicore_fifo_push_blocking((uint32_t)mutable_data());
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
    Capsense capsense;
    capsense.reset();

    // Run muted for a little while while USB is being initialised
    for (int i = 0; i < 2048; i++)
    {
        sleep_ms(1);
        set_led(state);
    }
    
    // Loop forever
    while (true)
    {
        sleep_ms(10);
        state = capsense.capsense_button(0.5);
        set_led(state);
    }
}