file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(CrossConnectionTest ${SOURCES})
target_link_libraries(CrossConnectionTest PRIVATE RakNet)
