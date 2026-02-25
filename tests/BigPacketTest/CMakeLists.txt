file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(BigPacketTest ${SOURCES})
target_link_libraries(BigPacketTest PRIVATE RakNet)
