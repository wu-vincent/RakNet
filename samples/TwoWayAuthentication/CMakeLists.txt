file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(TwoWayAuthentication ${SOURCES})
target_link_libraries(TwoWayAuthentication PRIVATE RakNet)
