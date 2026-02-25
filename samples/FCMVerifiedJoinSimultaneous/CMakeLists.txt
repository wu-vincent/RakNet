file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(FCMVerifiedJoinSimultaneous ${SOURCES})
target_link_libraries(FCMVerifiedJoinSimultaneous PRIVATE RakNet)
