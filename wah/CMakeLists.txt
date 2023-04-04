include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(wah
  wah.cpp
)

target_link_libraries(wah
                      pico_stdlib
                      tone_ale)

target_include_directories(wah PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(wah)
