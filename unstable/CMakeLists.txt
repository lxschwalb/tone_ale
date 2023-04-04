include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(unstable
unstable.cpp
)

target_link_libraries(unstable
                      pico_stdlib
                      tone_ale)

target_include_directories(unstable PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(unstable)
