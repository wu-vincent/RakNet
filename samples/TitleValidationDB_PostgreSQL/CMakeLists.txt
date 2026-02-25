file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
add_executable(TitleValidationDB_PostgreSQL ${SOURCES})
target_link_libraries(TitleValidationDB_PostgreSQL PRIVATE RakNet)
