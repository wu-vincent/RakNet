add_executable(MasterServer
    MasterClient.cpp MasterClient.h
    MasterClientMain.cpp
    MasterCommon.h mastercommon.cpp
    MasterServer.cpp MasterServer.h
    MasterServerMain.cpp
)
target_link_libraries(MasterServer PRIVATE RakNet)
