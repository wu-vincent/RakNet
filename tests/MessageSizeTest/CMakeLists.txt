file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(MessageSizeTest ${SOURCES})
target_link_libraries(MessageSizeTest PRIVATE RakNet)
