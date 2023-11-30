set(EXAMPLE_BIN "client")

add_executable(${EXAMPLE_BIN} client.cpp) 

target_link_libraries(client ${ECHO_WAVE_LIB}) # Need to link the proto library built
 
