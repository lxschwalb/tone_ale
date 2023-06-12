include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(amen_beat
amen_beat.cpp
)

target_link_libraries(amen_beat
                      pico_stdlib
                      tone_ale)

target_include_directories(amen_beat PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(amen_beat)
