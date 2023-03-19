include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

add_executable(audio_interface audio_interface.cpp)

target_include_directories(audio_interface PUBLIC
        "${PROJECT_SOURCE_DIR}/tone_ale_common"
        )

target_link_libraries(audio_interface PUBLIC pico_stdlib tone_ale tone_ale_usb pico_multicore)

pico_add_extra_outputs(audio_interface)