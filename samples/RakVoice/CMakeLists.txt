if(NOT TARGET LibRakVoice)
    message(STATUS "LibRakVoice not available -- skipping RakVoice sample")
    return()
endif()

add_executable(RakVoice main.cpp)
target_link_libraries(RakVoice PRIVATE RakNet LibRakVoice portaudio::portaudio)
