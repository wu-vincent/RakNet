file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Flow_Control_Test ${SOURCES})
target_link_libraries(Flow_Control_Test PRIVATE RakNet)
