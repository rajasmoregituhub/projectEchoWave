set(EXAMPLE_BIN "server")

add_executable(${EXAMPLE_BIN} server.cpp) 

target_link_libraries(server ${ECHO_WAVE_LIB}) # Need to link the proto library built
 
