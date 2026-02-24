find_package(Boost QUIET)
if(NOT Boost_FOUND)
    message(STATUS "Boost not found -- RPC3 extension will not be built")
    return()
endif()

file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(LibRPC3 STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(LibRPC3
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" ${Boost_INCLUDE_DIRS}
)
target_link_libraries(LibRPC3 PUBLIC RakNetLibStatic ${Boost_LIBRARIES})
set_target_properties(LibRPC3 PROPERTIES FOLDER "DependentExtensions")
if(MSVC)
    target_compile_options(LibRPC3 PRIVATE /W0)
else()
    target_compile_options(LibRPC3 PRIVATE -w)
endif()
