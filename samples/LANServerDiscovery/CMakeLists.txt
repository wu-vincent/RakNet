file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(LANServerDiscovery ${SOURCES})
target_link_libraries(LANServerDiscovery PRIVATE RakNet)
