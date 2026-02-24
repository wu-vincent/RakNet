set(XML_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/XML")
set(UPNP_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/UPNP")

file(GLOB XML_FILES CONFIGURE_DEPENDS "${XML_DIR}/*.cpp" "${XML_DIR}/*.h")
file(GLOB UPNP_FILES CONFIGURE_DEPENDS "${UPNP_DIR}/*.cpp" "${UPNP_DIR}/*.h")

raknet_add_sample(${CMAKE_CURRENT_SOURCE_DIR}
    EXTRA_SOURCES ${XML_FILES} ${UPNP_FILES}
    EXTRA_INCLUDES "${XML_DIR}" "${UPNP_DIR}"
    FOLDER "Samples/NAT Punchthrough"
)
