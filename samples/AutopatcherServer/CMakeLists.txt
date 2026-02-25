if(NOT TARGET AutopatcherPostgreRepository)
    message(STATUS "AutopatcherPostgreRepository not available -- skipping AutopatcherServer sample")
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

add_executable(AutopatcherServer_PostgreSQL
    AutopatcherServerTest.cpp
    ${AUTOSRC}
    ${BZSRC}
)
target_include_directories(AutopatcherServer_PostgreSQL PRIVATE
    "${AUTOPATCHER_DIR}"
    "${BZIP2_DIR}"
)
target_link_libraries(AutopatcherServer_PostgreSQL PRIVATE
    RakNet
    AutopatcherPostgreRepository
)
if(NOT WIN32)
    target_link_libraries(AutopatcherServer_PostgreSQL PRIVATE LibAutopatcher LibPostgreSQLInterface)
endif()
