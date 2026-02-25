add_library(LibRakVoice STATIC RakVoice.h RakVoice.cpp)
target_include_directories(LibRakVoice PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LibRakVoice PUBLIC RakNet Speex::speex)
set_target_properties(LibRakVoice PROPERTIES FOLDER "extensions")
