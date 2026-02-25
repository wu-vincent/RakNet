find_package(miniupnpc REQUIRED)

set(XML_DIR "${CMAKE_SOURCE_DIR}/extensions/XML")

file(GLOB XML_FILES CONFIGURE_DEPENDS "${XML_DIR}/*.cpp" "${XML_DIR}/*.h")

add_executable(NATCompleteClient main.cpp ${XML_FILES})
target_link_libraries(NATCompleteClient PRIVATE RakNet miniupnpc::miniupnpc)
target_include_directories(NATCompleteClient PRIVATE "${XML_DIR}")
