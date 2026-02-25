file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(FlowControlTest ${SOURCES})
target_link_libraries(FlowControlTest PRIVATE RakNet)
