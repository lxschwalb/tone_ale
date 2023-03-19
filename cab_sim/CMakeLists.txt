include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
add_executable(cab_sim
cab_sim.cpp
)

target_link_libraries(cab_sim
                      pico_stdlib
                      tone_ale)

target_include_directories(cab_sim PUBLIC
                          "${PROJECT_SOURCE_DIR}/tone_ale"
                          )

pico_add_extra_outputs(cab_sim)
