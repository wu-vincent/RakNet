file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(Fully_Connected_Mesh ${SOURCES})
target_link_libraries(Fully_Connected_Mesh PRIVATE RakNet)
