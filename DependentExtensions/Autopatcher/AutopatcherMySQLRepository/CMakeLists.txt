file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(AutoPatcherMySQLRepository STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(AutoPatcherMySQLRepository
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(AutoPatcherMySQLRepository
    PUBLIC RakNetLibStatic LibAutopatcher LibMySQLInterface
)
set_target_properties(AutoPatcherMySQLRepository PROPERTIES FOLDER "DependentExtensions")
if(MSVC)
    target_compile_options(AutoPatcherMySQLRepository PRIVATE /W0)
else()
    target_compile_options(AutoPatcherMySQLRepository PRIVATE -w)
endif()
