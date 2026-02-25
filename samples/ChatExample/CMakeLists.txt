add_executable(ChatServer ChatExampleServer.cpp)
target_link_libraries(ChatServer PRIVATE RakNet)

add_executable(ChatClient ChatExampleClient.cpp)
target_link_libraries(ChatClient PRIVATE RakNet)
