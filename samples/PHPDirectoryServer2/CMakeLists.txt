file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(PHPDirectoryServer2 ${SOURCES})
target_link_libraries(PHPDirectoryServer2 PRIVATE RakNet)
