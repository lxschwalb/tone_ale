include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(line_select
line_select.cpp
)

target_link_libraries(line_select
                      pico_stdlib
                      tone_ale)

target_include_directories(line_select PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(line_select)
