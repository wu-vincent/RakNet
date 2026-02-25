option(RAKNET_EXTENSION_USEMYSQL "Enable extensions using MySQL" ON)
option(RAKNET_EXTENSION_USEPOSTGRESQL "Enable extensions using PostgreSQL" ON)
option(RAKNET_EXTENSION_USEBOOST "Enable extensions using Boost" ON)
option(RAKNET_EXTENSION_USESPEEX "Enable extensions using Speex" ON)
option(RAKNET_EXTENSION_USEIRRLICHT "Enable extensions using Irrlicht" ON)
option(RAKNET_EXTENSION_USEIRRKLANG "Enable extensions using IrrKlang" ON)
option(RAKNET_EXTENSION_USEOGRE3D "Enable extensions using Ogre3D" ON)
option(RAKNET_EXTENSION_Autopatcher "Enable Autopatcher extension" ON)

find_package(BZip2 REQUIRED)
# Alias so existing targets (LibAutopatcher) can keep linking LibBZip2
if(NOT TARGET LibBZip2)
    add_library(LibBZip2 ALIAS BZip2::BZip2)
endif()

# Bundled XML library
file(GLOB XML_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/XML/*.cpp")
file(GLOB XML_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/XML/*.h")
add_library(RakNetXML STATIC ${XML_SOURCES} ${XML_HEADERS})
target_include_directories(RakNetXML PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/XML")
target_link_libraries(RakNetXML PRIVATE RakNet)
set_target_properties(RakNetXML PROPERTIES FOLDER "extensions")
if(MSVC)
    target_compile_options(RakNetXML PRIVATE /W0)
else()
    target_compile_options(RakNetXML PRIVATE -w)
endif()

find_package(portaudio REQUIRED)
if(NOT TARGET LibPortAudioV18_1)
    add_library(LibPortAudioV18_1 ALIAS portaudio::portaudio)
endif()

find_package(Speex REQUIRED)
if(NOT TARGET LibSpeex)
    add_library(LibSpeex ALIAS Speex::speex)
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

if(RAKNET_EXTENSION_USESPEEX)
    add_subdirectory(RakVoice)
endif()

# Irrlicht Demo
if(RAKNET_EXTENSION_USEIRRLICHT AND RAKNET_EXTENSION_USEIRRKLANG)
    add_subdirectory(IrrlichtDemo)
endif()

# Ogre3D Demo
if(RAKNET_EXTENSION_USEOGRE3D)
    add_subdirectory(Ogre3DInterpDemo)
endif()
