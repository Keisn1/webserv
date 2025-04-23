#include <ConfigParser.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class ServerConfigTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove("configTest");
    }

    ServerConfig parseConfig(const std::string& content) {
        std::ofstream file("configTest");
        file << content;
        file.close();
        ConfigParser parser("configTest");
        return parser.getServerConfig();
    }
};

TEST_F(ServerConfigTest, BasicServerConfiguration) {
    ServerConfig config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www/html;\n"
        "}\n"
    );

    EXPECT_EQ(config.listen.at("0.0.0.0"), 80);
    ASSERT_EQ(config.serverNames.size(), 1);
    EXPECT_EQ(config.serverNames[0], "example.com");
    EXPECT_EQ(config.root, "/var/www/html");
    EXPECT_TRUE(config.locations.empty());
}

TEST_F(ServerConfigTest, ServerWithLocationBlock) {
    ServerConfig config = parseConfig(
        "server {\n"
        "    listen 8080;\n"
        "    server_name example.com;\n"
        "    root /var/www/html;\n"
        "    location /images/ {\n"
        "        root /var/www/images;\n"
        "        allow_methods GET HEAD;\n"
        "    }\n"
        "}\n"
    );

    EXPECT_EQ(config.listen.at("0.0.0.0"), 8080);
    ASSERT_EQ(config.serverNames.size(), 1);
    EXPECT_EQ(config.serverNames[0], "example.com");
    EXPECT_EQ(config.root, "/var/www/html");
    ASSERT_EQ(config.locations.size(), 1);
    const LocationConfig& loc = config.locations[0];
    EXPECT_EQ(loc.prefix, "/images/");
    EXPECT_EQ(loc.root, "/var/www/images");
    ASSERT_THAT(loc.methods, testing::ElementsAre("GET", "HEAD"));
}

TEST_F(ServerConfigTest, HandlesClientMaxBodySize) {
    ServerConfig config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www/html;\n"
        "    server_name example.com;\n"
        "    client_max_body_size 10m;\n"
        "}\n"
    );

    EXPECT_EQ(config.listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config.serverNames[0], "example.com");
    EXPECT_EQ(config.root, "/var/www/html");
    EXPECT_EQ(config.clientMaxBody, 10 * 1024 * 1024);
}

TEST_F(ServerConfigTest, MultipleListenDirectives) {
    ServerConfig config = parseConfig(
        "server {\n"
        "    root /var/www/html;\n"
        "    server_name example.com;\n"
        "    listen 127.0.0.1:80;\n"
        "    listen [::1]:80;\n"
        "}\n"
    );

    EXPECT_EQ(config.serverNames[0], "example.com");
    EXPECT_EQ(config.root, "/var/www/html");
    EXPECT_EQ(config.listen.at("127.0.0.1"), 80);
    EXPECT_EQ(config.listen.at("::1"), 80);
}

TEST_F(ServerConfigTest, ThrowsOnInvalidDirective) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    invalid_directive;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, ThrowsOnMissingRequiredDirective) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    root /var/www;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, HandlesEmptyServerBlock) {
    EXPECT_THROW(
        parseConfig("server {}\n"),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, HandlesMultipleServerNames) {
    ServerConfig config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "}\n"
    );

    EXPECT_EQ(config.listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config.root, "/var/www");
    ASSERT_EQ(config.serverNames.size(), 2);
    EXPECT_EQ(config.serverNames[0], "example.com");
    EXPECT_EQ(config.serverNames[1], "www.example.com");
}