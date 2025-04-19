#include "test_ServerFixtures.h"
#include "test_main.h"
#include "gtest/gtest.h"

TEST_P(ServerWithMockLoggerParametrizedPortTest, connectionTest) {
    std::vector<std::string> listeningPorts = GetParam();
    for (size_t i = 0; i < listeningPorts.size(); i++)
        testMultipleConnectionsWithLogging(_logger, listeningPorts[i], 100);
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerWithMockLoggerParametrizedPortTest,
                         ::testing::Values(std::vector<std::string>{"8080"}, std::vector<std::string>{"8080", "8081"},
                                           std::vector<std::string>{"8080", "8081", "8082"}));

// // // TODO: make proper integration tests with either golang or python
// // // No parallel stuff in CPP
TEST_P(ServerTestWoMockLogging, connectionTest) {
    std::vector<std::string> listeningPorts = GetParam();
    for (size_t i = 0; i < listeningPorts.size(); i++)
        testMultipleConnections(listeningPorts[i], 10);
}

INSTANTIATE_TEST_SUITE_P(ServerTests, ServerTestWoMockLogging,
                         ::testing::Values(std::vector<std::string>{"8080"}, std::vector<std::string>{"8080", "8081"},
                                           std::vector<std::string>{"8080", "8081", "8082"}));
