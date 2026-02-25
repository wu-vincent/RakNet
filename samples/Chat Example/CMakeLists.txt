add_executable(ChatServer "Chat Example Server.cpp")
target_link_libraries(ChatServer PRIVATE RakNet)

add_executable(ChatClient "Chat Example Client.cpp")
target_link_libraries(ChatClient PRIVATE RakNet)
