include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(fuzz
  fuzz.cpp
)

target_link_libraries(fuzz
                      pico_stdlib
                      common)

target_include_directories(fuzz PUBLIC
                          "${PROJECT_SOURCE_DIR}/common"
                          )

pico_add_extra_outputs(fuzz)
