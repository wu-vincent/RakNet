find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(tests
    BigPacketTest.cpp
    BurstTest.cpp
    CloudTest.cpp
    ComprehensiveTest.cpp
    CrossConnectionTest.cpp
    DroppedConnectionTest.cpp
    MessageSizeTest.cpp
    OfflineMessagesTest.cpp
    ReliableOrderedTest.cpp
    RelayPluginTest.cpp
    ServerClientTest.cpp
    StatisticsHistoryTest.cpp
    ThreadTest.cpp
)
target_link_libraries(tests PRIVATE RakNet GTest::gtest_main)

gtest_discover_tests(tests)
