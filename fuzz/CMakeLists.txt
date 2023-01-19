if (TARGET tinyusb_device)

  include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
  add_executable(fuzz
    fuzz.cpp
  )

  target_link_libraries(fuzz
                        pico_stdlib
                        hardware_clocks
                        common)

  target_include_directories(fuzz PUBLIC
                            "${PROJECT_SOURCE_DIR}/common"
                            )

  # enable usb output, disable uart output
  pico_enable_stdio_usb(fuzz 1)
  pico_enable_stdio_uart(fuzz 0)

  pico_add_extra_outputs(fuzz)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building fuzz because TinyUSB submodule is not initialized in the SDK")
endif()