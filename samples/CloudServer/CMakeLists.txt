file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(CloudServer ${SOURCES})
target_link_libraries(CloudServer PRIVATE RakNet)
