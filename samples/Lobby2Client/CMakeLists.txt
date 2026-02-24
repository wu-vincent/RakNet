set(LOBBY2_DIR "${CMAKE_SOURCE_DIR}/DependentExtensions/Lobby2")

file(GLOB LOBBYFILES CONFIGURE_DEPENDS "${LOBBY2_DIR}/*.cpp" "${LOBBY2_DIR}/*.h")

add_executable(Lobby2Client
    Lobby2ClientSample.cpp
    ${LOBBYFILES}
)
target_include_directories(Lobby2Client PRIVATE
    "${LOBBY2_DIR}"
    "${LOBBY2_DIR}/Rooms"
)
target_link_libraries(Lobby2Client PRIVATE RakNet)
set_target_properties(Lobby2Client PROPERTIES FOLDER "Samples/Lobby2")
if(MSVC)
    target_compile_definitions(Lobby2Client PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    target_compile_options(Lobby2Client PRIVATE /W0)
else()
    target_compile_options(Lobby2Client PRIVATE -w)
endif()
