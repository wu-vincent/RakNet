set(AUTOPATCHER_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/Autopatcher")
set(BZIP2_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/bzip2-1.0.6")

file(GLOB AUTOSRC CONFIGURE_DEPENDS "${AUTOPATCHER_DIR}/*.cpp" "${AUTOPATCHER_DIR}/*.h")
list(FILTER AUTOSRC EXCLUDE REGEX "AutopatcherServer\\.(cpp|h)$")

file(GLOB BZSRC CONFIGURE_DEPENDS "${BZIP2_DIR}/*.c" "${BZIP2_DIR}/*.h")
list(FILTER BZSRC EXCLUDE REGEX "(dlltest|mk251|bzip2recover|bzip2)\\.c$")

add_executable(AutopatcherClient
    AutopatcherClientTest.cpp
    ${AUTOSRC}
    ${BZSRC}
)
target_include_directories(AutopatcherClient PRIVATE
    "${AUTOPATCHER_DIR}"
    "${BZIP2_DIR}"
)
target_link_libraries(AutopatcherClient PRIVATE RakNet)
