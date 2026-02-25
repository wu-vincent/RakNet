find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(tests
    BurstTest.cpp
    CrossConnectionTest.cpp
    MessageSizeTest.cpp
    OfflineMessagesTest.cpp
    ReliableOrderedTest.cpp
    StatisticsHistoryTest.cpp
    ThreadTest.cpp
)
target_link_libraries(tests PRIVATE RakNet GTest::gtest_main)

gtest_discover_tests(tests)
