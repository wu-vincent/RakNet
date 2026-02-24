find_package(FMODEx QUIET)
if(NOT FMODEX_FOUND)
    message(STATUS "FMODEx not found -- RakVoiceFMOD sample will not be built")
    return()
endif()

if(WIN32)
    file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.cpp"
    )
    file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.h"
    )

    add_executable(RakVoiceFMOD ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
    target_link_libraries(RakVoiceFMOD PRIVATE RakNetLibStatic LibSpeex ${FMODEX_LIBRARY})
    target_include_directories(RakVoiceFMOD PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/include"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/win32"
        ${FMODEX_INCLUDE_DIR}
    )
    target_compile_definitions(RakVoiceFMOD PRIVATE HAVE_CONFIG_H _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    target_compile_options(RakVoiceFMOD PRIVATE /W0)
    set_target_properties(RakVoiceFMOD PROPERTIES FOLDER "Samples/Voice")
else()
    find_package(PortAudio QUIET)
    if(NOT PORTAUDIO_FOUND)
        message(STATUS "PortAudio not found -- RakVoiceFMOD sample will not be built")
        return()
    endif()

    file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    add_executable(RakVoiceFMOD ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
    target_include_directories(RakVoiceFMOD PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        ${PORTAUDIO_INCLUDE_DIR}
        ${FMODEX_INCLUDE_DIR}
    )
    target_link_libraries(RakVoiceFMOD PRIVATE RakNetLibStatic ${PORTAUDIO_LIBRARIES} ${FMODEX_LIBRARY})
    if(TARGET LibRakVoice)
        target_link_libraries(RakVoiceFMOD PRIVATE LibRakVoice)
    endif()
    target_compile_options(RakVoiceFMOD PRIVATE -w)
    set_target_properties(RakVoiceFMOD PROPERTIES FOLDER "Samples/Voice")
endif()
