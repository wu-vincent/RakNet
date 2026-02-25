file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Encryption ${SOURCES})
target_link_libraries(Encryption PRIVATE RakNet)
