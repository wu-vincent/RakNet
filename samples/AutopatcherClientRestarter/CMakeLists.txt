file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(AutopatcherClientRestarter ${SOURCES})
target_link_libraries(AutopatcherClientRestarter PRIVATE RakNet)
