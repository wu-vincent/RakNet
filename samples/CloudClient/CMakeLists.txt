file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(CloudClient ${SOURCES})
target_link_libraries(CloudClient PRIVATE RakNet)
