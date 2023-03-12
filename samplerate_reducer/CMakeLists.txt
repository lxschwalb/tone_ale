include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(samplerate_reducer
  samplerate_reducer.cpp
)

target_link_libraries(samplerate_reducer
                      pico_stdlib
                      common)

target_include_directories(samplerate_reducer PUBLIC
                          "${PROJECT_SOURCE_DIR}/common"
                          )

pico_add_extra_outputs(samplerate_reducer)
