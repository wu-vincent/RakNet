file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(FCMHostSimultaneous ${SOURCES})
target_link_libraries(FCMHostSimultaneous PRIVATE RakNet)
