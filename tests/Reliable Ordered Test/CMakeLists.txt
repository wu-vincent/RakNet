file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Reliable_Ordered_Test ${SOURCES})
target_link_libraries(Reliable_Ordered_Test PRIVATE RakNet)
