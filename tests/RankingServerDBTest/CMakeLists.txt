file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(RankingServerDBTest ${SOURCES})
target_link_libraries(RankingServerDBTest PRIVATE RakNet)
