set(XML_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/XML")
set(UPNP_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/UPNP")

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp *.c *.h)
file(GLOB XML_FILES CONFIGURE_DEPENDS "${XML_DIR}/*.cpp" "${XML_DIR}/*.h")
file(GLOB UPNP_FILES CONFIGURE_DEPENDS "${UPNP_DIR}/*.cpp" "${UPNP_DIR}/*.h")

add_executable(NATCompleteClient ${SOURCES} ${XML_FILES} ${UPNP_FILES})
target_link_libraries(NATCompleteClient PRIVATE RakNet)
target_include_directories(NATCompleteClient PRIVATE "${XML_DIR}" "${UPNP_DIR}")
