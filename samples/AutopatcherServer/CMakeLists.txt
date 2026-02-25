if(NOT TARGET AutopatcherPostgreRepository)
    message(STATUS "AutopatcherPostgreRepository not available -- skipping AutopatcherServer sample")
    return()
endif()

set(AUTOPATCHER_DIR "${CMAKE_SOURCE_DIR}/extensions/Autopatcher")

set(AUTOSRC
    "${AUTOPATCHER_DIR}/AutopatcherServer.cpp"
    "${AUTOPATCHER_DIR}/AutopatcherServer.h"
    "${AUTOPATCHER_DIR}/MemoryCompressor.cpp"
    "${AUTOPATCHER_DIR}/CreatePatch.cpp"
)

add_executable(AutopatcherServer_PostgreSQL
    AutopatcherServerTest.cpp
    ${AUTOSRC}
)
target_include_directories(AutopatcherServer_PostgreSQL PRIVATE "${AUTOPATCHER_DIR}")
target_link_libraries(AutopatcherServer_PostgreSQL PRIVATE
    RakNet BZip2::BZip2 AutopatcherPostgreRepository
)
if(NOT WIN32)
    target_link_libraries(AutopatcherServer_PostgreSQL PRIVATE LibAutopatcher LibPostgreSQLInterface)
endif()
