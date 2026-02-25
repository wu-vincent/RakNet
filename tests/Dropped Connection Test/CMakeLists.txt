file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Dropped_Connection_Test ${SOURCES})
target_link_libraries(Dropped_Connection_Test PRIVATE RakNet)
