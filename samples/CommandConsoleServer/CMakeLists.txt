file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(CommandConsoleServer ${SOURCES})
target_link_libraries(CommandConsoleServer PRIVATE RakNet)
