file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(FCMHost ${SOURCES})
target_link_libraries(FCMHost PRIVATE RakNet)
