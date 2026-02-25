find_package(FMODEx QUIET)
if(NOT FMODEX_FOUND)
    message(STATUS "FMODEx not found -- RakVoiceFMOD sample will not be built")
    return()
endif()

if(WIN32)
    add_executable(RakVoiceFMOD
        FMODVoiceAdapter.cpp FMODVoiceAdapter.h
        main.cpp
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.cpp"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.h"
    )
    target_link_libraries(RakVoiceFMOD PRIVATE RakNet LibSpeex ${FMODEX_LIBRARY})
    target_include_directories(RakVoiceFMOD PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/include"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/win32"
        ${FMODEX_INCLUDE_DIR}
    )
    target_compile_definitions(RakVoiceFMOD PRIVATE HAVE_CONFIG_H)
else()
    find_package(PortAudio QUIET)
    if(NOT PORTAUDIO_FOUND)
        message(STATUS "PortAudio not found -- RakVoiceFMOD sample will not be built")
        return()
    endif()

    add_executable(RakVoiceFMOD
        FMODVoiceAdapter.cpp FMODVoiceAdapter.h
        main.cpp
    )
    target_include_directories(RakVoiceFMOD PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        ${PORTAUDIO_INCLUDE_DIR}
        ${FMODEX_INCLUDE_DIR}
    )
    target_link_libraries(RakVoiceFMOD PRIVATE RakNet ${PORTAUDIO_LIBRARIES} ${FMODEX_LIBRARY})
    if(TARGET LibRakVoice)
        target_link_libraries(RakVoiceFMOD PRIVATE LibRakVoice)
    endif()
endif()
