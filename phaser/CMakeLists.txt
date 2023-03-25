include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(phaser
phaser.cpp
)

target_link_libraries(phaser
                      pico_stdlib
                      tone_ale)

target_include_directories(phaser PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(phaser)
