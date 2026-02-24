if(WIN32)
    file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.cpp"
    )
    file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.h"
    )

    add_executable(RakVoice ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
    target_link_libraries(RakVoice PRIVATE RakNet LibPortAudioV18_1 LibSpeex)
    target_include_directories(RakVoice PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/portaudio_v18_1/pa_common"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/include"
        "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/win32"
    )
    target_compile_definitions(RakVoice PRIVATE HAVE_CONFIG_H _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    target_compile_options(RakVoice PRIVATE /W0)
    set_target_properties(RakVoice PROPERTIES FOLDER "Samples/Voice")
else()
    find_package(PortAudio QUIET)
    if(NOT PORTAUDIO_FOUND)
        message(STATUS "PortAudio not found -- RakVoice sample will not be built")
        return()
    endif()

    file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    add_executable(RakVoice ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
    target_include_directories(RakVoice PRIVATE
        "${CMAKE_SOURCE_DIR}/DependentExtensions"
        ${PORTAUDIO_INCLUDE_DIR}
    )
    target_link_libraries(RakVoice PRIVATE RakNet ${PORTAUDIO_LIBRARIES})
    if(TARGET LibRakVoice)
        target_link_libraries(RakVoice PRIVATE LibRakVoice)
    endif()
    target_compile_options(RakVoice PRIVATE -w)
    set_target_properties(RakVoice PROPERTIES FOLDER "Samples/Voice")
endif()
