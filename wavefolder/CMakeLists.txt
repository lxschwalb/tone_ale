include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(wavefolder
  wavefolder.cpp
)

target_link_libraries(wavefolder
                      pico_stdlib
                      tone_ale)

target_include_directories(wavefolder PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(wavefolder)
