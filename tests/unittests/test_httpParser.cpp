#include "HttpParser.h"
#include "HttpRequest.h"
#include "IHttpParser.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

struct TestHttpParserParams {
    int bufLen;
    int wantReady;
    int wantError;
    struct HttpRequest wantReq;
    std::string req;
};

class TestHttpParser : public ::testing::TestWithParam<TestHttpParserParams> {};

void assertEqualHttpRequest(struct HttpRequest wantReq,
                            struct HttpRequest gotReq) {
    ASSERT_EQ(wantReq.uri, gotReq.uri);
}

TEST_P(TestHttpParser, testParsing) {
    HttpParser httpParser;
    struct TestHttpParserParams params = GetParam();

    // cutting the string into parts
    std::vector<std::string> chunks;
    for (std::size_t i = 0; i < params.req.length(); i += params.bufLen) {
        chunks.push_back(params.req.substr(i, params.bufLen));
    }

    // feeding the parser
    for (std::size_t i = 0; i < chunks.size(); i++)
        httpParser.feed(chunks[i].c_str(), params.bufLen);

    ASSERT_EQ(params.wantError, httpParser.error());
    ASSERT_EQ(params.wantReady, httpParser.ready());
    if (httpParser.ready())
        assertEqualHttpRequest(params.wantReq, httpParser.getRequest());
}

INSTANTIATE_TEST_SUITE_P(
    httpParser, TestHttpParser,
    ::testing::Values(TestHttpParserParams{4, 1, 0, {""}, R"(GET / HTTP/1.1
host localhost

)"},

                      TestHttpParserParams{4, 1, 0, {""}, R"(POST / HTTP/1.1
host www

)"}));
