file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(TeamManager ${SOURCES})
target_link_libraries(TeamManager PRIVATE RakNet)
