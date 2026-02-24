option(RAKNET_EXTENSION_USEMYSQL "Enable extensions using MySQL" ON)
option(RAKNET_EXTENSION_USEPOSTGRESQL "Enable extensions using PostgreSQL" ON)
option(RAKNET_EXTENSION_USEBOOST "Enable extensions using Boost" ON)
option(RAKNET_EXTENSION_USESPEEX "Enable extensions using Speex" ON)
option(RAKNET_EXTENSION_USEIRRLICHT "Enable extensions using Irrlicht" ON)
option(RAKNET_EXTENSION_USEIRRKLANG "Enable extensions using IrrKlang" ON)
option(RAKNET_EXTENSION_USEOGRE3D "Enable extensions using Ogre3D" ON)
option(RAKNET_EXTENSION_Autopatcher "Enable Autopatcher extension" ON)

# Bundled bzip2 library
file(GLOB BZIP2_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/bzip2-1.0.6/*.c")
list(FILTER BZIP2_SOURCES EXCLUDE REGEX "(dlltest|mk251|bzip2recover|bzip2)\\.c$")
file(GLOB BZIP2_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/bzip2-1.0.6/*.h")
add_library(LibBZip2 STATIC ${BZIP2_SOURCES} ${BZIP2_HEADERS})
target_include_directories(LibBZip2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/bzip2-1.0.6")
set_target_properties(LibBZip2 PROPERTIES FOLDER "DependentExtensions")
if(MSVC)
    target_compile_options(LibBZip2 PRIVATE /W0)
else()
    target_compile_options(LibBZip2 PRIVATE -w)
endif()

# Bundled XML library
file(GLOB XML_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/XML/*.cpp")
file(GLOB XML_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/XML/*.h")
add_library(RakNetXML STATIC ${XML_SOURCES} ${XML_HEADERS})
target_include_directories(RakNetXML PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/XML")
target_link_libraries(RakNetXML PRIVATE RakNetLibStatic)
set_target_properties(RakNetXML PROPERTIES FOLDER "DependentExtensions")
if(MSVC)
    target_compile_options(RakNetXML PRIVATE /W0)
else()
    target_compile_options(RakNetXML PRIVATE -w)
endif()

# Bundled PortAudio (Windows only)
if(WIN32 AND NOT CMAKE_GENERATOR STREQUAL "MSYS Makefiles")
    file(GLOB PA_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/portaudio_v18_1/pa_common/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/portaudio_v18_1/pa_win_wmme/*.c"
    )
    file(GLOB PA_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/portaudio_v18_1/pa_common/*.h"
    )
    add_library(LibPortAudioV18_1 STATIC ${PA_SOURCES} ${PA_HEADERS})
    target_include_directories(LibPortAudioV18_1 PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/portaudio_v18_1/pa_common"
    )
    set_target_properties(LibPortAudioV18_1 PROPERTIES FOLDER "DependentExtensions")
    if(MSVC)
        target_compile_options(LibPortAudioV18_1 PRIVATE /W0)
    else()
        target_compile_options(LibPortAudioV18_1 PRIVATE -w)
    endif()
endif()

# Bundled Speex (Windows only)
if(WIN32)
    file(GLOB SPEEX_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/libspeex/*.c")
    file(GLOB SPEEX_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/include/speex/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/libspeex/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/win32/*.h"
    )
    list(FILTER SPEEX_SOURCES EXCLUDE REGEX "pcm_wrapper\\.c$")
    add_library(LibSpeex STATIC ${SPEEX_SOURCES} ${SPEEX_HEADERS})
    target_include_directories(LibSpeex PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/speex-1.1.12/win32"
    )
    target_compile_definitions(LibSpeex PRIVATE HAVE_CONFIG_H)
    set_target_properties(LibSpeex PROPERTIES FOLDER "DependentExtensions")
    if(MSVC)
        target_compile_options(LibSpeex PRIVATE /W0)
    else()
        target_compile_options(LibSpeex PRIVATE -w)
    endif()
endif()

# MySQL Interface
if(RAKNET_EXTENSION_USEMYSQL)
    add_subdirectory(MySQLInterface)
endif()

# PostgreSQL Interface
if(RAKNET_EXTENSION_USEPOSTGRESQL)
    add_subdirectory(PostgreSQLInterface)
endif()

# Autopatcher
if(RAKNET_EXTENSION_Autopatcher)
    add_subdirectory(Autopatcher)
endif()

# RPC3 (requires Boost)
if(RAKNET_EXTENSION_USEBOOST)
    add_subdirectory(RPC3)
endif()

# LibRakVoice (non-Windows, requires Speex)
if(NOT WIN32 AND RAKNET_EXTENSION_USESPEEX)
    find_package(Speex QUIET)
    if(SPEEX_FOUND)
        add_library(LibRakVoice STATIC RakVoice.h RakVoice.cpp)
        target_include_directories(LibRakVoice PRIVATE ${SPEEX_INCLUDE_DIRS})
        target_link_libraries(LibRakVoice PUBLIC RakNetLibStatic ${SPEEX_LIBRARIES})
        set_target_properties(LibRakVoice PROPERTIES FOLDER "DependentExtensions")
    else()
        message(STATUS "Speex not found -- LibRakVoice will not be built")
    endif()
endif()

# Irrlicht Demo
if(RAKNET_EXTENSION_USEIRRLICHT AND RAKNET_EXTENSION_USEIRRKLANG)
    add_subdirectory(IrrlichtDemo)
endif()

# Ogre3D Demo
if(RAKNET_EXTENSION_USEOGRE3D)
    add_subdirectory(Ogre3DInterpDemo)
endif()
