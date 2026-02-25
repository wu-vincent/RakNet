file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(CommandConsoleClient ${SOURCES})
target_link_libraries(CommandConsoleClient PRIVATE RakNet)
