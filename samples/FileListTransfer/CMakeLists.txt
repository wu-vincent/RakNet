file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(FileListTransfer ${SOURCES})
target_link_libraries(FileListTransfer PRIVATE RakNet)
