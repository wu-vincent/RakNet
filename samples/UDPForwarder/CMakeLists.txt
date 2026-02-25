file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(UDPForwarder ${SOURCES})
target_link_libraries(UDPForwarder PRIVATE RakNet)
