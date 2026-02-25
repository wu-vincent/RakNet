option(RAKNET_EXTENSION_USEMYSQL "Enable extensions using MySQL" ON)
option(RAKNET_EXTENSION_USEPOSTGRESQL "Enable extensions using PostgreSQL" ON)
option(RAKNET_EXTENSION_USEBOOST "Enable extensions using Boost" ON)
option(RAKNET_EXTENSION_USESPEEX "Enable extensions using Speex" ON)
option(RAKNET_EXTENSION_USEIRRLICHT "Enable extensions using Irrlicht" ON)
option(RAKNET_EXTENSION_USEIRRKLANG "Enable extensions using IrrKlang" ON)
option(RAKNET_EXTENSION_USEOGRE3D "Enable extensions using Ogre3D" ON)
option(RAKNET_EXTENSION_Autopatcher "Enable Autopatcher extension" ON)

find_package(BZip2 REQUIRED)

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
