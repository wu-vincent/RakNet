file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(ServerClientTest2 ${SOURCES})
target_link_libraries(ServerClientTest2 PRIVATE RakNet)
