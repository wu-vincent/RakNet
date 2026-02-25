file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(OfflineMessagesTest ${SOURCES})
target_link_libraries(OfflineMessagesTest PRIVATE RakNet)
