file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(DirectoryDeltaTransfer ${SOURCES})
target_link_libraries(DirectoryDeltaTransfer PRIVATE RakNet)
