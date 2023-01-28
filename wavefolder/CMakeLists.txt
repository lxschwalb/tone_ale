include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(wavefolder
  wavefolder.cpp
)

target_link_libraries(wavefolder
                      pico_stdlib
                      common)

target_include_directories(wavefolder PUBLIC
                          "${PROJECT_SOURCE_DIR}/common"
                          )

pico_add_extra_outputs(wavefolder)
