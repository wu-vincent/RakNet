file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(ReliableOrderedTest ${SOURCES})
target_link_libraries(ReliableOrderedTest PRIVATE RakNet)
