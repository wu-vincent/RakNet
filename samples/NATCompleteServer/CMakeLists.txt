file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(NATCompleteServer ${SOURCES})
target_link_libraries(NATCompleteServer PRIVATE RakNet)
