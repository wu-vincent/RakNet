find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(tests
    CrossConnectionTest.cpp
    MessageSizeTest.cpp
    OfflineMessagesTest.cpp
)
target_link_libraries(tests PRIVATE RakNet GTest::gtest_main)

gtest_discover_tests(tests)
