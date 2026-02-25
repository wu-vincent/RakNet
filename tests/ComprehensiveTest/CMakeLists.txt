file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(ComprehensiveTest ${SOURCES})
target_link_libraries(ComprehensiveTest PRIVATE RakNet)
