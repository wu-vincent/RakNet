cmake_minimum_required(VERSION 3.5)
project(AutopatcherClientRestarter)
include_directories(${RAKNETHEADERFILES} ./) 
add_executable(AutopatcherClientRestarter "main.cpp" "readme.txt")
target_link_libraries(AutopatcherClientRestarter ${RAKNET_COMMON_LIBS})
VSUBFOLDER(AutopatcherClientRestarter "Samples/AutoPatcher/Client")





