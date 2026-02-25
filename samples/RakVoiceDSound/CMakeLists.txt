if(NOT WIN32)
    return()
endif()

if(NOT DEFINED ENV{DXSDK_DIR} OR "$ENV{DXSDK_DIR}" STREQUAL "")
    message(STATUS "DXSDK_DIR not set -- RakVoiceDSound sample will not be built")
    return()
endif()

if(NOT TARGET LibRakVoice)
    message(STATUS "LibRakVoice not available -- skipping RakVoiceDSound sample")
    return()
endif()

set(DXSDK_DIR "$ENV{DXSDK_DIR}")

add_executable(RakVoiceDSound
    DSoundVoiceAdapter.cpp DSoundVoiceAdapter.h
    main.cpp
)
target_link_libraries(RakVoiceDSound PRIVATE RakNet LibRakVoice Speex::speex winmm
    "${DXSDK_DIR}lib/x86/Dsound.lib"
    "${DXSDK_DIR}lib/x86/Dxguid.lib"
    "${DXSDK_DIR}lib/x86/Dxerr.lib"
)
target_include_directories(RakVoiceDSound PRIVATE
    "${CMAKE_SOURCE_DIR}/DependentExtensions"
    "${DXSDK_DIR}include"
)
target_compile_definitions(RakVoiceDSound PRIVATE _UNICODE UNICODE)
