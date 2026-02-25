file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(TestDLL ${SOURCES})
target_link_libraries(TestDLL PRIVATE RakNet)
