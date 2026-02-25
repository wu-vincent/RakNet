file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(ComprehensivePCGame ${SOURCES})
target_link_libraries(ComprehensivePCGame PRIVATE RakNet)
