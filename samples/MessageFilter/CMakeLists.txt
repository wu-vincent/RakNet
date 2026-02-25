file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(MessageFilter ${SOURCES})
target_link_libraries(MessageFilter PRIVATE RakNet)
