file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(LoopbackPerformanceTest ${SOURCES})
target_link_libraries(LoopbackPerformanceTest PRIVATE RakNet)
