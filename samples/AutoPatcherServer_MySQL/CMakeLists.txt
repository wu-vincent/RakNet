if(NOT TARGET AutoPatcherMySQLRepository)
    message(STATUS "AutoPatcherMySQLRepository not available -- skipping AutoPatcherServer_MySQL sample")
    return()
endif()

set(AUTOPATCHER_DIR "${CMAKE_SOURCE_DIR}/extensions/Autopatcher")

set(AUTOSRC
    "${AUTOPATCHER_DIR}/AutopatcherServer.cpp"
    "${AUTOPATCHER_DIR}/AutopatcherServer.h"
    "${AUTOPATCHER_DIR}/MemoryCompressor.cpp"
    "${AUTOPATCHER_DIR}/CreatePatch.cpp"
)

add_executable(AutoPatcherServer_MySQL
    AutopatcherServerTest_MySQL.cpp
    ${AUTOSRC}
)
target_include_directories(AutoPatcherServer_MySQL PRIVATE "${AUTOPATCHER_DIR}")
target_link_libraries(AutoPatcherServer_MySQL PRIVATE
    RakNet BZip2::BZip2 AutoPatcherMySQLRepository
)
if(NOT WIN32)
    target_link_libraries(AutoPatcherServer_MySQL PRIVATE LibAutopatcher LibMySQLInterface)
endif()
