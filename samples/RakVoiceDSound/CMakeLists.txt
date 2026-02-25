if(NOT WIN32)
    return()
endif()

# DirectSound requires the DirectX SDK
if(NOT DEFINED ENV{DXSDK_DIR} OR "$ENV{DXSDK_DIR}" STREQUAL "")
    message(STATUS "DXSDK_DIR not set -- RakVoiceDSound sample will not be built")
    return()
endif()

set(DXSDK_DIR "$ENV{DXSDK_DIR}")

file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.cpp"
)
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_SOURCE_DIR}/DependentExtensions/RakVoice.h"
)

add_executable(RakVoiceDSound ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_link_libraries(RakVoiceDSound PRIVATE RakNet LibSpeex winmm
    "${DXSDK_DIR}lib/x86/Dsound.lib"
    "${DXSDK_DIR}lib/x86/Dxguid.lib"
    "${DXSDK_DIR}lib/x86/Dxerr.lib"
)
target_include_directories(RakVoiceDSound PRIVATE
    "${CMAKE_SOURCE_DIR}/DependentExtensions"
    "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/include"
    "${CMAKE_SOURCE_DIR}/DependentExtensions/speex-1.1.12/win32"
    "${DXSDK_DIR}include"
)
target_compile_definitions(RakVoiceDSound PRIVATE HAVE_CONFIG_H _UNICODE UNICODE)
