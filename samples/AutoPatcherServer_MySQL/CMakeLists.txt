if(NOT TARGET AutoPatcherMySQLRepository)
    message(STATUS "AutoPatcherMySQLRepository not available -- skipping AutoPatcherServer_MySQL sample")
    return()
endif()

set(AUTOPATCHER_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/Autopatcher")
set(BZIP2_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/bzip2-1.0.6")

set(AUTOSRC
    "${AUTOPATCHER_DIR}/AutopatcherServer.cpp"
    "${AUTOPATCHER_DIR}/AutopatcherServer.h"
    "${AUTOPATCHER_DIR}/MemoryCompressor.cpp"
    "${AUTOPATCHER_DIR}/CreatePatch.cpp"
)

if(WIN32)
    file(GLOB BZSRC CONFIGURE_DEPENDS "${BZIP2_DIR}/*.c" "${BZIP2_DIR}/*.h")
    list(FILTER BZSRC EXCLUDE REGEX "(dlltest|mk251|bzip2recover|bzip2)\\.c$")
endif()

add_executable(AutoPatcherServer_MySQL
    AutopatcherServerTest_MySQL.cpp
    ${AUTOSRC}
    ${BZSRC}
)
target_include_directories(AutoPatcherServer_MySQL PRIVATE
    "${AUTOPATCHER_DIR}"
    "${BZIP2_DIR}"
)
target_link_libraries(AutoPatcherServer_MySQL PRIVATE
    RakNet
    AutoPatcherMySQLRepository
)
if(NOT WIN32)
    target_link_libraries(AutoPatcherServer_MySQL PRIVATE LibAutopatcher LibMySQLInterface)
endif()
