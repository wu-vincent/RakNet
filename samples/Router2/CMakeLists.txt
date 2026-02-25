file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Router2 ${SOURCES})
target_link_libraries(Router2 PRIVATE RakNet)
