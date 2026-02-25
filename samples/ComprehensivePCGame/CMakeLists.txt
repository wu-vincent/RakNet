find_package(miniupnpc REQUIRED)

add_executable(ComprehensivePCGame ComprehensivePCGame.cpp)
target_link_libraries(ComprehensivePCGame PRIVATE RakNet miniupnpc::miniupnpc)
