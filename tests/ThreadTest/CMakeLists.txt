file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(ThreadTest ${SOURCES})
target_link_libraries(ThreadTest PRIVATE RakNet)
