if (TARGET tinyusb_device)

  include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

  add_library(common common_pins.cpp common_capsense.cpp common_i2cv.cpp)

  pico_generate_pio_header(common ${PROJECT_SOURCE_DIR}/common/capsense.pio)
  pico_generate_pio_header(common ${PROJECT_SOURCE_DIR}/common/i2cv.pio)

  target_link_libraries(common
                        pico_stdlib
                        hardware_dma
                        hardware_irq
                        hardware_pio)

endif()