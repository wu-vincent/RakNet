file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(RPC4 ${SOURCES})
target_link_libraries(RPC4 PRIVATE RakNet)
