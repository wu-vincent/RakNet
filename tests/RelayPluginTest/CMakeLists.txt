file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(RelayPluginTest ${SOURCES})
target_link_libraries(RelayPluginTest PRIVATE RakNet)
