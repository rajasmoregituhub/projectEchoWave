set(EXAMPLE_BIN "${CMAKE_PROJECT_NAME}_example")
add_executable(${EXAMPLE_BIN} main.cpp)
target_link_libraries(${EXAMPLE_BIN} ${ECHO_WAVE_LIB}) # Need to link the proto library built