file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(PacketLogger ${SOURCES})
target_link_libraries(PacketLogger PRIVATE RakNet)
