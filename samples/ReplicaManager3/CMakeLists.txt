file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(ReplicaManager3 ${SOURCES})
target_link_libraries(ReplicaManager3 PRIVATE RakNet)
