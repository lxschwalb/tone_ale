include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(reverb
reverb.cpp
)

target_link_libraries(reverb
                      pico_stdlib
                      tone_ale)

target_include_directories(reverb PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(reverb)
