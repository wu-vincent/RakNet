file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(CloudTest ${SOURCES})
target_link_libraries(CloudTest PRIVATE RakNet)
