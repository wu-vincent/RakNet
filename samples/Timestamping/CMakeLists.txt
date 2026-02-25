file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Timestamping ${SOURCES})
target_link_libraries(Timestamping PRIVATE RakNet)
