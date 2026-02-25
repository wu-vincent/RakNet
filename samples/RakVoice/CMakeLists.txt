if(WIN32)
    add_executable(RakVoice
        main.cpp
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.cpp"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.h"
    )
    target_link_libraries(RakVoice PRIVATE RakNet LibPortAudioV18_1 LibSpeex)
    target_include_directories(RakVoice PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/portaudio_v18_1/pa_common"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/include"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/win32"
    )
    target_compile_definitions(RakVoice PRIVATE HAVE_CONFIG_H)
else()
    find_package(PortAudio QUIET)
    if(NOT PORTAUDIO_FOUND)
        message(STATUS "PortAudio not found -- RakVoice sample will not be built")
        return()
    endif()

    add_executable(RakVoice main.cpp)
    target_include_directories(RakVoice PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        ${PORTAUDIO_INCLUDE_DIR}
    )
    target_link_libraries(RakVoice PRIVATE RakNet ${PORTAUDIO_LIBRARIES})
    if(TARGET LibRakVoice)
        target_link_libraries(RakVoice PRIVATE LibRakVoice)
    endif()
endif()
