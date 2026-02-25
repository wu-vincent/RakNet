file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(MasterServer ${SOURCES})
target_link_libraries(MasterServer PRIVATE RakNet)
