#include "tusb.h"
#include "common.h"

#define BUFFSIZE    16
#define SAMPLE_RATE 48000 // has to be 48k when usb audio in use
#define SYSTEM_CLK  270000000

void interrupt_service_routine() {
    juggle_buffers();

    int32_t *buff = mutable_data();

    for (int i = 0; i < BUFFSIZE; i++)
    {
        buff[i] = buff[i]<<8;
        usb_audio_buff_thing(&buff[i]);
    }
}

int main()
{
    int32_t data_buff[BUFFSIZE*3];
    set_sys_clock_khz(SYSTEM_CLK/1000, true);
    common_pins_setup();
    common_capsense_setup();
    common_clk_setup(SAMPLE_RATE, SYSTEM_CLK);
    common_i2cv_setup(data_buff, BUFFSIZE, interrupt_service_routine);

    tusb_init();

    // Loop forever
    while (true)
    {
        tud_task();
    }
}