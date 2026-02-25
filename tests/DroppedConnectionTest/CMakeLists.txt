file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(DroppedConnectionTest ${SOURCES})
target_link_libraries(DroppedConnectionTest PRIVATE RakNet)
