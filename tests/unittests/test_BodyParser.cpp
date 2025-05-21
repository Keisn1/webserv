#include "BodyParser.h"
#include "Connection.h"
#include "test_main.h"
#include <gtest/gtest.h>
#include <string>

// TODO: check Content equals 0
// TODO: check neither Content-Length nor Transfer-Encoding given
TEST(BodyParserTest, bodyWithoutOverlap) {
    BodyParser* bodyPrsr = new BodyParser();
    int contentLength = 12345;
    std::string body = getRandomString(contentLength);

    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    size_t pos = 0;
    while (!conn->_bodyFinished) {
        int chunkSize = getRandomNumber(10, 50);
        std::string bodyChunk = body.substr(pos, chunkSize);
        pos += chunkSize;

        conn->_readBuf.assign(bodyChunk);
        bodyPrsr->parse(conn);
        EXPECT_EQ(conn->_tempBody, bodyChunk);
        EXPECT_EQ(conn->_readBuf.size(), 0);
    }

    EXPECT_TRUE(conn->_bodyFinished);

    delete conn;
    delete bodyPrsr;
}

class BodyParserTestContentLength : public ::testing::TestWithParam< int > {
  protected:
    BodyParser* bodyPrsr;

    void SetUp() override { bodyPrsr = new BodyParser(); }

    void TearDown() override { delete bodyPrsr; }
};

TEST_P(BodyParserTestContentLength, BodyWithOverlap) {
    const int connectionCount = GetParam();
    std::vector< Connection* > connections;
    std::vector< std::string > bodies;
    std::vector< std::string > bytesList;
    std::vector< size_t > positions;

    // Setup all connections
    for (int i = 0; i < connectionCount; i++) {
        Connection* conn = new Connection({}, -1, "", NULL, NULL);
        int contentLength = 9876;
        conn->_request.headers["content-length"] = std::to_string(contentLength);
        conn->_bodyFinished = false;

        std::string body = getRandomString(contentLength);
        std::string bytes = body + getRandomString(10000 - contentLength);

        connections.push_back(conn);
        bodies.push_back(body);
        bytesList.push_back(bytes);
        positions.push_back(0);
    }

    // Process all connections until complete
    bool allDone = false;
    while (!allDone) {
        allDone = true;

        for (size_t i = 0; i < connections.size(); i++) {
            size_t& pos = positions[i];
            if (pos < bodies[i].length()) {
                allDone = false;

                int chunkSize = getRandomNumber(10, 50);
                std::string bytesChunk = bytesList[i].substr(pos, chunkSize);
                connections[i]->_readBuf.assign(bytesChunk);

                bodyPrsr->parse(connections[i]);

                std::string restOfBody = bodies[i].substr(pos);
                pos += chunkSize;

                if (pos < bodies[i].length()) {
                    EXPECT_EQ(connections[i]->_tempBody, bytesChunk);
                    EXPECT_EQ(connections[i]->_readBuf.size(), 0);
                } else {
                    EXPECT_EQ(connections[i]->_tempBody, restOfBody);
                    EXPECT_EQ(connections[i]->_readBuf.size(), chunkSize - restOfBody.size());
                }
            }
        }
    }

    // Cleanup
    for (auto conn : connections) {
        delete conn;
    }
}

// Run the test with 1, 5, and 10 concurrent connections
INSTANTIATE_TEST_SUITE_P(ConcurrentConnections, BodyParserTestContentLength, ::testing::Values(1, 5, 10));

TEST(BodyParserTest, respectsClientMaxBodySize) {
    BodyParser* bodyPrsr = new BodyParser();

    // Setup connection with a RouteConfig that has a limited clientMaxBody
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    const size_t maxBodySize = 100;
    conn->route.cfg.clientMaxBody = maxBodySize;

    // Set a content-length that exceeds the max body size
    const int contentLength = maxBodySize + 50;
    conn->_request.headers["content-length"] = std::to_string(contentLength);
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    // Create a body that's larger than allowed
    std::string body = getRandomString(contentLength);

    // First chunk within limits
    int firstChunkSize = 80;
    conn->_readBuf.assign(body.substr(0, firstChunkSize));
    bodyPrsr->parse(conn);

    // This should be accepted
    EXPECT_EQ(conn->getState(), Connection::SendResponse);
    EXPECT_EQ(conn->_response.statusCode, 413);
    EXPECT_EQ(conn->_response.statusMessage, "Content Too Large");

    delete conn;
    delete bodyPrsr;
}

TEST(BodyParserTest, noContentLengthSetsBodyToFinished) {
    BodyParser* bodyPrsr = new BodyParser();

    // Setup connection with a RouteConfig that has a limited clientMaxBody
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    const size_t maxBodySize = 100;
    conn->route.cfg.clientMaxBody = maxBodySize;

    // Set a content-length that exceeds the max body size
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    bodyPrsr->parse(conn);

    // This should be accepted
    EXPECT_TRUE(conn->_bodyFinished);
    EXPECT_EQ(conn->getState(), Connection::Handling);

    delete conn;
    delete bodyPrsr;
}

TEST(BodyParserTest, transferEncodingTestForNoBody) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["transfer-encoding"] = "chunked";
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;

    bodyPrsr->parse(conn);

    EXPECT_TRUE(conn->_bodyFinished);


    delete conn;
    delete bodyPrsr;
}
// for this test, the content-length is not equal to the actrual length of the content
TEST(BodyParserTest, transferEncodingTestForIncorrectBody1) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["transfer-encoding"] = "chunked";
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;
    conn->_readBuf.assign("6\r\nhello\r\n0\r\n\r\n");

    bodyPrsr->parse(conn);

    EXPECT_EQ(conn->getState(), Connection::SendResponse);
    EXPECT_EQ(conn->_response.statusCode, 400);
    EXPECT_EQ(conn->_response.statusMessage, "Bad Request");

    delete conn;
    delete bodyPrsr;
}

// for this test, there is no "0\r\n\r\n" ending
TEST(BodyParserTest, transferEncodingTestForIncorrectBody2) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["transfer-encoding"] = "chunked";
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;
    conn->_readBuf.assign("6\r\nhello!\r\n");

    bodyPrsr->parse(conn);

    EXPECT_EQ(conn->getState(), Connection::SendResponse);
    EXPECT_EQ(conn->_response.statusCode, 400);
    EXPECT_EQ(conn->_response.statusMessage, "Bad Request");

    delete conn;
    delete bodyPrsr;
}

TEST(BodyParserTest, transferEncodingTestForCorrectBody) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["transfer-encoding"] = "chunked";
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;
    conn->_readBuf.assign("6\r\nhello!\r\n0\r\n\r\n");

    bodyPrsr->parse(conn);

    EXPECT_EQ(conn->_tempBody, "hello!");
    EXPECT_TRUE(conn->_bodyFinished);

    delete conn;
    delete bodyPrsr;
}

TEST(BodyParserTest, transferEncodingTestForCorrectBodyButPartly) {
    BodyParser* bodyPrsr = new BodyParser();
    Connection* conn = new Connection({}, -1, "", NULL, NULL);
    conn->_request.headers["transfer-encoding"] = "chunked";
    conn->setState(Connection::Handling);
    conn->_bodyFinished = false;
    std::string body("6\r\nhello \r\n6\r\nworld!\r\n0\r\n\r\n");
    
    size_t pos = 0;
    // int chunkSize = 6;
    int chunkSize = getRandomNumber(5, 10);
    std::cout << chunkSize << std::endl;
    while (pos <= body.size()) {
        std::string bodyChunk = body.substr(pos, chunkSize);
        pos += chunkSize;

        conn->_readBuf.assign(bodyChunk);
        bodyPrsr->parse(conn);
    }

    EXPECT_EQ(conn->_tempBody, "hello world!");
    EXPECT_TRUE(conn->_bodyFinished);

    delete conn;
    delete bodyPrsr;
}
