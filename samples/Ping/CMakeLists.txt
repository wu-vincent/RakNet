file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Ping ${SOURCES})
target_link_libraries(Ping PRIVATE RakNet)
