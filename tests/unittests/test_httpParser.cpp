#include "HttpParser.h"
#include "IHttpParser.h"
#include "test_main.h"
#include <gtest/gtest.h>

// struct TestHttpParserParams {};

// class TestHttpParser : public ::testing::TestWithParam<TestHttpParserParams>
// {};

TEST(HttpParserTestSuite, firstTest) {
    IHttpParser *httpParser = new HttpParser;
}
