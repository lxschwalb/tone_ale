include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(rig
rig.cpp
)

target_link_libraries(rig
                      pico_stdlib
                      common)

target_include_directories(rig PUBLIC
                          "${PROJECT_SOURCE_DIR}/common"
                          )

pico_add_extra_outputs(rig)
