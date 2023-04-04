include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(lowpass
  lowpass.cpp
)

target_link_libraries(lowpass
                      pico_stdlib
                      tone_ale)

target_include_directories(lowpass PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(lowpass)
