find_package(Boost QUIET)
if(NOT Boost_FOUND)
    message(STATUS "Boost not found -- RPC3 sample will not be built")
    return()
endif()

set(RPC3_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/RPC3")

if(WIN32)
    set(RPC3_PLUGIN_FILES
        "${RPC3_DIR}/RPC3.cpp"
        "${RPC3_DIR}/RPC3.h"
        "${RPC3_DIR}/RPC3_Boost.h"
    )
    raknet_add_sample(${CMAKE_CURRENT_SOURCE_DIR}
        EXTRA_SOURCES ${RPC3_PLUGIN_FILES}
        EXTRA_INCLUDES "${RPC3_DIR}" ${Boost_INCLUDE_DIRS}
    )
else()
    raknet_add_sample(${CMAKE_CURRENT_SOURCE_DIR}
        EXTRA_INCLUDES "${RPC3_DIR}" ${Boost_INCLUDE_DIRS}
        EXTRA_LIBS LibRPC3
    )
endif()
