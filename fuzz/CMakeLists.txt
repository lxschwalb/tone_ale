include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(fuzz
  fuzz.cpp
)

target_link_libraries(fuzz
                      pico_stdlib
                      tone_ale)

target_include_directories(fuzz PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(fuzz)
