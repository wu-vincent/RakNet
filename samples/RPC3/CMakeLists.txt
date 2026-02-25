find_package(Boost QUIET)
if(NOT Boost_FOUND)
    message(STATUS "Boost not found -- RPC3 sample will not be built")
    return()
endif()

set(RPC3_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/RPC3")
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)

if(WIN32)
    add_executable(RPC3 ${SOURCES}
        "${RPC3_DIR}/RPC3.cpp"
        "${RPC3_DIR}/RPC3.h"
        "${RPC3_DIR}/RPC3_Boost.h"
    )
    target_link_libraries(RPC3 PRIVATE RakNet)
    target_include_directories(RPC3 PRIVATE "${RPC3_DIR}" ${Boost_INCLUDE_DIRS})
else()
    add_executable(RPC3 ${SOURCES})
    target_link_libraries(RPC3 PRIVATE RakNet LibRPC3)
    target_include_directories(RPC3 PRIVATE "${RPC3_DIR}" ${Boost_INCLUDE_DIRS})
endif()
