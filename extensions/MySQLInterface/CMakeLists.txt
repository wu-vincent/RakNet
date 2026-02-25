find_package(MySQL QUIET)
if(NOT MYSQL_FOUND)
    message(STATUS "MySQL not found -- MySQLInterface will not be built")
    return()
endif()

file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(LibMySQLInterface STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(LibMySQLInterface
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE ${MYSQL_INCLUDE_DIR}
)
target_link_libraries(LibMySQLInterface PUBLIC RakNet PRIVATE ${MYSQL_LIBRARIES})
set_target_properties(LibMySQLInterface PROPERTIES FOLDER "extensions")
if(MSVC)
    target_compile_options(LibMySQLInterface PRIVATE /W0)
else()
    target_compile_options(LibMySQLInterface PRIVATE -w)
endif()
