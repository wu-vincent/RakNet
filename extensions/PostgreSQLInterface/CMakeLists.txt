find_package(PostgreSQL QUIET)
if(NOT TARGET libpq::libpq)
    message(STATUS "PostgreSQL not found -- PostgreSQLInterface will not be built")
    return()
endif()

file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(LibPostgreSQLInterface STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(LibPostgreSQLInterface
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(LibPostgreSQLInterface PUBLIC RakNet PRIVATE libpq::libpq)
set_target_properties(LibPostgreSQLInterface PROPERTIES FOLDER "extensions")
if(MSVC)
    target_compile_options(LibPostgreSQLInterface PRIVATE /W0)
else()
    target_compile_options(LibPostgreSQLInterface PRIVATE -w)
endif()
