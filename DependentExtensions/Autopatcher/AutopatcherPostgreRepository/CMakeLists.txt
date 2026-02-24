file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(AutopatcherPostgreRepository STATIC ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(AutopatcherPostgreRepository
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(AutopatcherPostgreRepository
    PUBLIC RakNetLibStatic LibAutopatcher LibPostgreSQLInterface
)
set_target_properties(AutopatcherPostgreRepository PROPERTIES FOLDER "DependentExtensions")
if(MSVC)
    target_compile_options(AutopatcherPostgreRepository PRIVATE /W0)
else()
    target_compile_options(AutopatcherPostgreRepository PRIVATE -w)
endif()
