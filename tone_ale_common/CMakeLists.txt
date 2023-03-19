if (TARGET tinyusb_device)

  include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

  add_library(tone_ale tone_ale_pins.cpp tone_ale_capsense.cpp tone_ale_i2cv.cpp usb_audio.cpp)
  target_include_directories(tone_ale PUBLIC ${CMAKE_CURRENT_LIST_DIR})

  pico_generate_pio_header(tone_ale ${PROJECT_SOURCE_DIR}/tone_ale_common/capsense.pio)
  pico_generate_pio_header(tone_ale ${PROJECT_SOURCE_DIR}/tone_ale_common/i2cv.pio)

  target_link_libraries(tone_ale
                        pico_stdlib
                        hardware_dma
                        hardware_irq
                        hardware_pio
                        tinyusb_device
                        tinyusb_board
                        pico_unique_id)

endif()