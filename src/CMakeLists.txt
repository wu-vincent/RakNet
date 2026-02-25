include(GNUInstallDirs)

file(GLOB RAKNET_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB RAKNET_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/RakNet/*.h")

if(NOT WIN32)
    find_package(Threads REQUIRED)
endif()

find_package(OpenSSL QUIET)

# Single library target — BUILD_SHARED_LIBS controls static vs shared
add_library(RakNet ${RAKNET_SOURCES} ${RAKNET_HEADERS})
add_library(RakNet::RakNet ALIAS RakNet)

target_include_directories(RakNet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/RakNet>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/RakNet>
    PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(RakNet PRIVATE _RAKNET_DLL)
else()
    target_compile_definitions(RakNet PRIVATE _RAKNET_LIB)
endif()

if(MSVC)
    target_compile_definitions(RakNet PRIVATE
        _CRT_SECURE_NO_DEPRECATE
        _CRT_NONSTDC_NO_DEPRECATE
    )
    target_compile_options(RakNet PRIVATE /W0 /GS- /GR-)
else()
    target_compile_options(RakNet PRIVATE -w)
endif()

if(WIN32)
    target_link_libraries(RakNet PUBLIC ws2_32)
else()
    target_link_libraries(RakNet PUBLIC Threads::Threads)
endif()

if(OpenSSL_FOUND)
    target_link_libraries(RakNet PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

set_target_properties(RakNet PROPERTIES FOLDER "Library")

install(TARGETS RakNet
    EXPORT RakNetTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
