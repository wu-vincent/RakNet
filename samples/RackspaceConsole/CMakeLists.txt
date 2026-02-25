file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(RackspaceConsole ${SOURCES})
target_link_libraries(RackspaceConsole PRIVATE RakNet)
