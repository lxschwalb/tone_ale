include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(vibrato
vibrato.cpp
)

target_link_libraries(vibrato
                      pico_stdlib
                      tone_ale)

target_include_directories(vibrato PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(vibrato)
