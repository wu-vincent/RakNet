file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(StatisticsHistoryTest ${SOURCES})
target_link_libraries(StatisticsHistoryTest PRIVATE RakNet)
