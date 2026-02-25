set(AUTOPATCHER_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/Autopatcher")

file(GLOB AUTOSRC CONFIGURE_DEPENDS "${AUTOPATCHER_DIR}/*.cpp" "${AUTOPATCHER_DIR}/*.h")
list(FILTER AUTOSRC EXCLUDE REGEX "AutopatcherServer\\.(cpp|h)$")

add_executable(AutopatcherClient
    AutopatcherClientTest.cpp
    ${AUTOSRC}
)
target_include_directories(AutopatcherClient PRIVATE "${AUTOPATCHER_DIR}")
target_link_libraries(AutopatcherClient PRIVATE RakNet BZip2::BZip2)
