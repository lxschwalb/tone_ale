include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(delay
delay.cpp
)

target_link_libraries(delay
                      pico_stdlib
                      common)

target_include_directories(delay PUBLIC
                          "${PROJECT_SOURCE_DIR}/common"
                          )

pico_add_extra_outputs(delay)
