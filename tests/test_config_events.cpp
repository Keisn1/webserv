#include <ConfigParser.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class EventsConfigTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove("configTest.conf");
    }

    EventsConfig parseConfig(const std::string& content) {
        std::ofstream file("configTest.conf");
        file << content;
        file.close();
        ConfigParser parser("configTest.conf");
        return parser.getEventsConfig();
    }
};

TEST_F(EventsConfigTest, EventsCheck) {
    EventsConfig config = parseConfig(
        "events {\n"
        "    worker_connections 1024;"
        "    use epoll;"
        "}\n"
    );

    EXPECT_EQ(config.maxEvents, 1024);
    EXPECT_EQ(config.kernelMethod, "epoll");
}

TEST_F(EventsConfigTest, ValidEventsConfiguration) {
    EventsConfig config = parseConfig(
        "events {\n"
        "    worker_connections 512;\n"
        "    use kqueue;\n"
        "}\n"
    );

    EXPECT_EQ(config.maxEvents, 512);
    EXPECT_EQ(config.kernelMethod, "kqueue");
}

TEST_F(EventsConfigTest, DefaultValues) {
    EventsConfig config = parseConfig(
        "events {\n"
        "    # Empty block with defaults\n"
        "}\n"
    );

    EXPECT_EQ(config.maxEvents, 512);
    EXPECT_EQ(config.kernelMethod, "epoll");
}

TEST_F(EventsConfigTest, InvalidDirective) {
    EXPECT_THROW(
        parseConfig(
            "events {\n"
            "    worker_connections 1024;\n"
            "    invalid_directive;\n"
            "}\n"
        ), 
        std::runtime_error
    );
}

TEST_F(EventsConfigTest, MissingSemicolon) {
    EXPECT_THROW(
        parseConfig(
            "events {\n"
            "    worker_connections 1024\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(EventsConfigTest, InvalidKernelMethod) {
    EXPECT_THROW(
        parseConfig(
            "events {\n"
            "    use invalid_method;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(EventsConfigTest, NegativeWorkerConnections) {
    EXPECT_THROW(
        parseConfig(
            "events {\n"
            "    worker_connections -100;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(EventsConfigTest, ExceedMaximumWorkerConnections) {
    EXPECT_THROW(
        parseConfig(
            "events {\n"
            "    worker_connections 1025;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(EventsConfigTest, MultipleWorkerConnections) {
    EventsConfig config = parseConfig(
        "events {\n"
        "    worker_connections 1024;\n"
        "    worker_connections 512;\n"
        "}\n"
    );

    EXPECT_EQ(config.maxEvents, 512);
}

TEST_F(EventsConfigTest, CommentsAndWhitespace) {
    EventsConfig config = parseConfig(
        "events {\n"
        "    # Important comment\n"
        "    worker_connections    512   ;  \n"
        "    use   epoll ;  \n"
        "}\n"
    );

    EXPECT_EQ(config.maxEvents, 512);
    EXPECT_EQ(config.kernelMethod, "epoll");
}

TEST_F(EventsConfigTest, UnclosedEventsBlock) {
    EXPECT_THROW(
        parseConfig(
            "events {\n"
            "    worker_connections 1024;\n"
        ),
        std::runtime_error
    );
}

TEST_F(EventsConfigTest, MultipleEventsBlocks) {
    EXPECT_THROW(
        parseConfig(
            "events { worker_connections 1024; }\n"
            "events { worker_connections 512; }\n"
        ),
        std::runtime_error
    );
}
