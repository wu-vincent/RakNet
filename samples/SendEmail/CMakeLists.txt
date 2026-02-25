file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(SendEmail ${SOURCES})
target_link_libraries(SendEmail PRIVATE RakNet)
