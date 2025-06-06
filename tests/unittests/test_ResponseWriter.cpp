#include "ResponseWriter.h"
#include "test_main.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

HttpResponse newResponse(std::string body) {
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusMessage = "OK";
    if (!body.empty())
        resp.body = new StringBodyProvider(body);
    resp.contentLength = body.length();
    resp.version = "HTTP/1.1";
    return resp;
}

typedef struct ResponseWriterParams {
    std::string body;
    size_t maxSize;
} ResponseWriterParams;

class ResponseWriterTest : public testing::TestWithParam< struct ResponseWriterParams > {};

TEST_P(ResponseWriterTest, firstTest) {
    ResponseWriterParams params = GetParam();
    size_t maxSize = params.maxSize;

    char buffer[8192];
    HttpResponse resp = newResponse(params.body);

    std::string want = "HTTP/1.1 200 OK\r\n";
    want += "Content-Length: " + std::to_string(params.body.length()) +
            "\r\n"
            "\r\n" +
            params.body;

    IResponseWriter* wrtr = new ResponseWriter(resp);
    size_t writtenBytes = -1;
    std::string got = "";
    while (writtenBytes != 0) {
        writtenBytes = wrtr->write(buffer, maxSize);
        got += std::string(buffer, writtenBytes);
    }

    EXPECT_STREQ(want.c_str(), got.c_str());

    delete resp.body;
    delete wrtr;
}

INSTANTIATE_TEST_SUITE_P(maxSizes, ResponseWriterTest,
                         testing::Values(ResponseWriterParams{"pong", 1}, ResponseWriterParams{"pong", 2},
                                         ResponseWriterParams{"pong", 3}));

INSTANTIATE_TEST_SUITE_P(bodyEmpty, ResponseWriterTest, testing::Values(ResponseWriterParams{"", 1}));

INSTANTIATE_TEST_SUITE_P(msg100long, ResponseWriterTest,
                         testing::Values(ResponseWriterParams{getRandomString(100), 1},
                                         ResponseWriterParams{getRandomString(100), 2},
                                         ResponseWriterParams{getRandomString(100), 3}));

INSTANTIATE_TEST_SUITE_P(
    msg1000long, ResponseWriterTest,
    testing::Values(ResponseWriterParams{getRandomString(1000), 1}, ResponseWriterParams{getRandomString(1000), 2},
                    ResponseWriterParams{getRandomString(1000), 3}, ResponseWriterParams{getRandomString(1000), 4},
                    ResponseWriterParams{getRandomString(1000), 10}, ResponseWriterParams{getRandomString(1000), 100},
                    ResponseWriterParams{getRandomString(1000), 1000},
                    ResponseWriterParams{getRandomString(1000), 2000}));
