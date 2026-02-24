include(GNUInstallDirs)

file(GLOB RAKNET_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Source/*.cpp")
file(GLOB RAKNET_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Source/*.h")

if(NOT WIN32)
    find_package(Threads REQUIRED)
endif()

# Shared configuration function
function(raknet_configure_target TARGET)
    target_include_directories(${TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Source>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/RakNet>
        PRIVATE
            "${CMAKE_SOURCE_DIR}/DependentExtensions/openssl-1.0.0d/include"
            "${CMAKE_SOURCE_DIR}/DependentExtensions"
    )

    if(MSVC)
        target_compile_definitions(${TARGET} PRIVATE
            _CRT_SECURE_NO_DEPRECATE
            _CRT_NONSTDC_NO_DEPRECATE
        )
        target_compile_options(${TARGET} PRIVATE /W0 /GS- /GR-)
    else()
        target_compile_options(${TARGET} PRIVATE -w)
    endif()

    if(WIN32)
        target_link_libraries(${TARGET} PUBLIC ws2_32)
    else()
        target_link_libraries(${TARGET} PUBLIC Threads::Threads)
    endif()
endfunction()

# Static library
if(RAKNET_ENABLE_STATIC)
    add_library(RakNetLibStatic STATIC ${RAKNET_SOURCES} ${RAKNET_HEADERS})
    raknet_configure_target(RakNetLibStatic)
    target_compile_definitions(RakNetLibStatic PRIVATE _RAKNET_LIB)
    add_library(RakNet::Static ALIAS RakNetLibStatic)
    add_library(RakNet::RakNet ALIAS RakNetLibStatic)
    set_target_properties(RakNetLibStatic PROPERTIES FOLDER "Library")

    install(TARGETS RakNetLibStatic
        EXPORT RakNetTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()

# Shared library
if(RAKNET_ENABLE_DLL)
    add_library(RakNetDLL SHARED ${RAKNET_SOURCES} ${RAKNET_HEADERS})
    raknet_configure_target(RakNetDLL)
    target_compile_definitions(RakNetDLL PRIVATE _RAKNET_DLL)
    add_library(RakNet::Shared ALIAS RakNetDLL)
    set_target_properties(RakNetDLL PROPERTIES FOLDER "Library")

    if(NOT RAKNET_ENABLE_STATIC)
        add_library(RakNet::RakNet ALIAS RakNetDLL)
    endif()

    install(TARGETS RakNetDLL
        EXPORT RakNetTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()
