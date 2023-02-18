#include "tusb.h"
#include "tusb_config.h"
#include "pico/unique_id.h"
#define ID_LEN  (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1)

#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (   USB_PID_BASE          \
                            | _PID_MAP(   CDC, 0)   \
                            | _PID_MAP(   MSC, 1)   \
                            | _PID_MAP(   HID, 2)   \
                            | _PID_MAP(  MIDI, 3)   \
                            | _PID_MAP( AUDIO, 4)   \
                            | _PID_MAP(VENDOR, 5)   \
                          )

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
#define CONFIG_TOTAL_LEN    	(TUD_CONFIG_DESC_LEN + CFG_TUD_AUDIO * TUD_AUDIO_DESC_LEN)

#define EPNUM_AUDIO_IN    0x02
#define EPNUM_AUDIO_OUT   0x01

uint8_t const desc_configuration[] =
{
    // Interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // string index overall, string index spk, string index mic, EP Out & EP In address
    TUD_AUDIO_DESCRIPTOR(2, 4, 5, EPNUM_AUDIO_OUT, EPNUM_AUDIO_IN | 0x80),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index; // for multiple configurations
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 },  // 0: is supported language is English (0x0409)
  "Tone Age Technology",          // 1: Manufacturer
  "Tone Ale",                     // 2: Product
  "666",                          // 3: Serials, should use chip ID
  "OUT",                          // 4: Audio Interface
  "IN",                           // 5: Audio Interface
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;

  uint8_t chr_count;

  if (index == 0)
  {
    //language
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }
  else
  {
    // Convert ASCII string into UTF-16

    if (!(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0]))) return NULL;

    const char* str;
    char id[ID_LEN];
    if (index == 3)
    {
      //serial
      pico_get_unique_board_id_string(id, ID_LEN);
      str = id;
    }
    else
    {
      //everything else
      str = string_desc_arr[index];
    }

    // Cap at max char
    chr_count = strlen(str);
    if (chr_count > 31) chr_count = 31;

    for (uint8_t i = 0; i < chr_count; i++)
    {
      _desc_str[1 + i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2 * chr_count + 2);

  return _desc_str;
}

#define MIC_BUF_SIZE    CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ
#define SPK_BUF_SIZE    CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ

// Buffer for microphone data
uint8_t usb_mic_buff[MIC_BUF_SIZE];
int32_t mic_buff_index = 0;

// Buffer for speaker data
uint8_t usb_spk_buff[SPK_BUF_SIZE];
int32_t spk_buff_index = 0;
int bytes_read;

void usb_audio_buff_thing(int32_t *buffi) {
    if (mic_buff_index < MIC_BUF_SIZE)
    {
        usb_mic_buff[mic_buff_index++] = *buffi;
        usb_mic_buff[mic_buff_index++] = (*buffi >> 8) & 0xFB; //TODO for some reason bit 2 alway has to be 0. Figure out why
        usb_mic_buff[mic_buff_index++] = *buffi >> 16;
        usb_mic_buff[mic_buff_index++] = *buffi >> 24;
    }
    if (spk_buff_index < bytes_read)
    {
        *buffi = (int32_t) usb_spk_buff[spk_buff_index++];
        *buffi |= (int32_t)usb_spk_buff[spk_buff_index++] << 8;
        *buffi |= (int32_t)usb_spk_buff[spk_buff_index++] << 16;
        *buffi |= (int32_t)usb_spk_buff[spk_buff_index++] << 24;
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Helper for clock get requests
static bool tud_audio_clock_get_request(uint8_t rhport, audio_control_request_t const *request)
{
    TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);

    if (request->bControlSelector == AUDIO_CS_CTRL_SAM_FREQ)
    {
        if (request->bRequest == AUDIO_CS_REQ_CUR)
        {
            audio_control_cur_4_t curf = { tu_htole32(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE) };
            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &curf, sizeof(curf));
        }
        if (request->bRequest == AUDIO_CS_REQ_RANGE)
        {
            audio_control_range_4_n_t(1) rangef = {.wNumSubRanges = tu_htole16(1)};
            rangef.subrange[0].bMin = CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE;
            rangef.subrange[0].bMax = CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE;
            rangef.subrange[0].bRes = 0;
            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &rangef, sizeof(rangef));
        }
    }
    if (request->bControlSelector == AUDIO_CS_CTRL_CLK_VALID)
    {
        if (request->bRequest == AUDIO_CS_REQ_CUR)
        {
            audio_control_cur_1_t cur_valid = { .bCur = 1 };
            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &cur_valid, sizeof(cur_valid));
        }
    }
    return false;
}

// Helper for clock set requests
static bool tud_audio_clock_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
    (void)rhport;

    TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);
    TU_VERIFY(request->bRequest == AUDIO_CS_REQ_CUR);

    if (request->bControlSelector == AUDIO_CS_CTRL_SAM_FREQ)
    {
        TU_VERIFY(request->wLength == sizeof(audio_control_cur_4_t));
        return true;
    }
    return false;
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
    audio_control_request_t *request = (audio_control_request_t *)p_request;
    if (request->bEntityID == UAC2_ENTITY_CLOCK)
    {
        return tud_audio_clock_get_request(rhport, request);
    }
    return false;
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf)
{
    audio_control_request_t const *request = (audio_control_request_t const *)p_request;
    if (request->bEntityID == UAC2_ENTITY_CLOCK)
    {
        return tud_audio_clock_set_request(rhport, request, buf);
    }
    return false;
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
    (void)rhport;
    (void)itf;
    (void)ep_in;
    (void)cur_alt_setting;

    tud_audio_write(usb_mic_buff, mic_buff_index);   //read from buffer, write to USB
    mic_buff_index = 0;

    return true;
}

bool tud_audio_rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes_received, uint8_t func_id, uint8_t ep_out, uint8_t cur_alt_setting)
{
    (void)rhport;
    (void)func_id;
    (void)ep_out;
    (void)cur_alt_setting;

    bytes_read = tud_audio_read(usb_spk_buff, n_bytes_received);     //read from USB, write to buffer
    spk_buff_index = 0;

    return true;
}
