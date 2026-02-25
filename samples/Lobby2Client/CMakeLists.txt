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
