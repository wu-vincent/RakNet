find_package(FMODEx QUIET)
if(NOT FMODEX_FOUND)
    message(STATUS "FMODEx not found -- RakVoiceFMOD sample will not be built")
    return()
endif()

if(NOT TARGET LibRakVoice)
    message(STATUS "LibRakVoice not available -- skipping RakVoiceFMOD sample")
    return()
endif()

add_executable(RakVoiceFMOD
    FMODVoiceAdapter.cpp FMODVoiceAdapter.h
    main.cpp
)
target_link_libraries(RakVoiceFMOD PRIVATE RakNet LibRakVoice Speex::speex ${FMODEX_LIBRARY})
target_include_directories(RakVoiceFMOD PRIVATE
    "${CMAKE_SOURCE_DIR}/DependentExtensions"
    ${FMODEX_INCLUDE_DIR}
)
