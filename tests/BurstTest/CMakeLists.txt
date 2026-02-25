file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(BurstTest ${SOURCES})
target_link_libraries(BurstTest PRIVATE RakNet)
