find_package(miniupnpc REQUIRED)

add_executable(NATCompleteClient main.cpp)
target_link_libraries(NATCompleteClient PRIVATE RakNet miniupnpc::miniupnpc)
