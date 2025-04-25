#include <ConfigParser.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class ServerConfigTest : public ::testing::Test {
protected:
    void TearDown() override {
        std::remove("configTest.conf");
    }

    std::vector<ServerConfig> parseConfig(const std::string& content) {
        std::ofstream file("configTest.conf");
        file << content;
        file.close();
        ConfigParser parser("configTest.conf");
        return parser.getServersConfig();
    }
};

TEST_F(ServerConfigTest, BasicServerConfiguration) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www/html;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    ASSERT_EQ(config[0].serverNames.size(), 1);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].common.root, "/var/www/html");
    EXPECT_TRUE(config[0].locations.empty());
}

TEST_F(ServerConfigTest, ServerWithLocationBlock) {
    std::vector<ServerConfig> config = parseConfig(
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

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 8080);
    ASSERT_EQ(config[0].serverNames.size(), 1);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].common.root, "/var/www/html");
    ASSERT_EQ(config[0].locations.size(), 1);
    const LocationConfig& loc = config[0].locations[0];
    EXPECT_EQ(loc.prefix, "/images/");
    EXPECT_EQ(loc.common.root, "/var/www/images");
    ASSERT_THAT(loc.common.allowMethods, testing::ElementsAre("GET", "HEAD"));
}

TEST_F(ServerConfigTest, HandlesClientMaxBodySize) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www/html;\n"
        "    server_name example.com;\n"
        "    client_max_body_size 10m;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].common.root, "/var/www/html");
    EXPECT_EQ(config[0].common.clientMaxBody, 10 * 1024 * 1024);
}

TEST_F(ServerConfigTest, MultipleListenDirectives) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    root /var/www/html;\n"
        "    server_name example.com;\n"
        "    listen 127.0.0.1:80;\n"
        "    listen [::1]:80;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].common.root, "/var/www/html");
    EXPECT_EQ(config[0].listen.at("127.0.0.1"), 80);
    EXPECT_EQ(config[0].listen.at("::1"), 80);
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
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].common.root, "/var/www");
    ASSERT_EQ(config[0].serverNames.size(), 2);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].serverNames[1], "www.example.com");
}

TEST_F(ServerConfigTest, HandleIndexFiles) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "    index index.html index.htm default.html;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].common.root, "/var/www");
    ASSERT_EQ(config[0].serverNames.size(), 2);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].serverNames[1], "www.example.com");
    ASSERT_EQ(config[0].common.index.size(), 3);
    EXPECT_EQ(config[0].common.index[0], "index.html");
    EXPECT_EQ(config[0].common.index[1], "index.htm");
    EXPECT_EQ(config[0].common.index[2], "default.html");
}

TEST_F(ServerConfigTest, HandleIndexFilesInLocationContext) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "    location /images/ {\n"
        "        index index.html index.htm default.html;\n"
        "    }\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].common.root, "/var/www");
    ASSERT_EQ(config[0].serverNames.size(), 2);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].serverNames[1], "www.example.com");
    const LocationConfig& loc = config[0].locations[0];
    ASSERT_EQ(loc.common.index.size(), 3);
    EXPECT_EQ(loc.common.index[0], "index.html");
    EXPECT_EQ(loc.common.index[1], "index.htm");
    EXPECT_EQ(loc.common.index[2], "default.html");
}

TEST_F(ServerConfigTest, HandleIndexFilesInMultipleLocationContext) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "    location /images/ {\n"
        "        index index.html index.htm default.html;\n"
        "    }\n"
        "    location /somewhere/ {\n"
        "        index another_index.html another_index.htm another_default.html;\n"
        "    }\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].common.root, "/var/www");
    ASSERT_EQ(config[0].serverNames.size(), 2);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].serverNames[1], "www.example.com");
    ASSERT_EQ(config[0].locations.size(), 2);
    const LocationConfig& loc1 = config[0].locations[0];
    EXPECT_EQ(loc1.prefix, "/images/");
    ASSERT_EQ(loc1.common.index.size(), 3);
    EXPECT_EQ(loc1.common.index[0], "index.html");
    EXPECT_EQ(loc1.common.index[1], "index.htm");
    EXPECT_EQ(loc1.common.index[2], "default.html");
    const LocationConfig& loc2 = config[0].locations[1];
    EXPECT_EQ(loc2.prefix, "/somewhere/");
    ASSERT_EQ(loc2.common.index.size(), 3);
    EXPECT_EQ(loc2.common.index[0], "another_index.html");
    EXPECT_EQ(loc2.common.index[1], "another_index.htm");
    EXPECT_EQ(loc2.common.index[2], "another_default.html");
}

TEST_F(ServerConfigTest, ThrowsOnInvalidHttpMethodInLocation) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "    location /images/ {\n"
        "        allow_methods GET SKIBIDI;\n"
        "        index index.html index.htm default.html;\n"
        "    }\n"
        "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, ThrowsOnInvalidHttpMethodInServer) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    allow_methods GET SKIBIDI;\n"
        "    server_name example.com www.example.com;\n"
        "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, HandleMultipleServerContextWithLocationsInside) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "    location /images/ {\n"
        "        index index.html index.htm default.html;\n"
        "    }\n"
        "}\n"
        "server {\n"
        "    listen 81;\n"
        "    root /car/www;\n"
        "    server_name nexample.com www.nexample.com;\n"
        "    location /nimages/ {\n"
        "        index pindex.html pindex.htm pefault.html;\n"
        "    }\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].common.root, "/var/www");
    ASSERT_EQ(config[0].serverNames.size(), 2);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].serverNames[1], "www.example.com");
    ASSERT_EQ(config[0].locations.size(), 1);
    const LocationConfig& loc1 = config[0].locations[0];
    EXPECT_EQ(loc1.prefix, "/images/");
    ASSERT_EQ(loc1.common.index.size(), 3);
    EXPECT_EQ(loc1.common.index[0], "index.html");
    EXPECT_EQ(loc1.common.index[1], "index.htm");
    EXPECT_EQ(loc1.common.index[2], "default.html");
    
    EXPECT_EQ(config[1].listen.at("0.0.0.0"), 81);
    EXPECT_EQ(config[1].common.root, "/car/www");
    ASSERT_EQ(config[1].serverNames.size(), 2);
    EXPECT_EQ(config[1].serverNames[0], "nexample.com");
    EXPECT_EQ(config[1].serverNames[1], "www.nexample.com");
    ASSERT_EQ(config[1].locations.size(), 1);
    const LocationConfig& loc2 = config[1].locations[0];
    EXPECT_EQ(loc2.prefix, "/nimages/");
    ASSERT_EQ(loc2.common.index.size(), 3);
    EXPECT_EQ(loc2.common.index[0], "pindex.html");
    EXPECT_EQ(loc2.common.index[1], "pindex.htm");
    EXPECT_EQ(loc2.common.index[2], "pefault.html");
}

TEST_F(ServerConfigTest, HandleMultipleServerContextWithMultipleLocationsInside) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    root /var/www;\n"
        "    server_name example.com www.example.com;\n"
        "    location /images/ {\n"
        "        index index.html index.htm default.html;\n"
        "    }\n"
        "    location /nimages/ {\n"
        "        index pindex.html pindex.htm pefault.html;\n"
        "    }\n"
        "}\n"
        "server {\n"
        "    listen 81;\n"
        "    root /car/www;\n"
        "    server_name nexample.com www.nexample.com;\n"
        "    location /nimages/ {\n"
        "        index pindex.html pindex.htm pefault.html;\n"
        "    }\n"
        "    location /images/ {\n"
        "        index index.html index.htm default.html;\n"
        "    }\n"
        "}\n"
    );

    EXPECT_EQ(config[0].listen.at("0.0.0.0"), 80);
    EXPECT_EQ(config[0].common.root, "/var/www");
    ASSERT_EQ(config[0].serverNames.size(), 2);
    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].serverNames[1], "www.example.com");
    ASSERT_EQ(config[0].locations.size(), 2);
    const LocationConfig& loc1 = config[0].locations[0];
    EXPECT_EQ(loc1.prefix, "/images/");
    ASSERT_EQ(loc1.common.index.size(), 3);
    EXPECT_EQ(loc1.common.index[0], "index.html");
    EXPECT_EQ(loc1.common.index[1], "index.htm");
    EXPECT_EQ(loc1.common.index[2], "default.html");
    const LocationConfig& loc2 = config[0].locations[1];
    EXPECT_EQ(loc2.prefix, "/nimages/");
    ASSERT_EQ(loc2.common.index.size(), 3);
    EXPECT_EQ(loc2.common.index[0], "pindex.html");
    EXPECT_EQ(loc2.common.index[1], "pindex.htm");
    EXPECT_EQ(loc2.common.index[2], "pefault.html");
    
    EXPECT_EQ(config[1].listen.at("0.0.0.0"), 81);
    EXPECT_EQ(config[1].common.root, "/car/www");
    ASSERT_EQ(config[1].serverNames.size(), 2);
    EXPECT_EQ(config[1].serverNames[0], "nexample.com");
    EXPECT_EQ(config[1].serverNames[1], "www.nexample.com");
    ASSERT_EQ(config[1].locations.size(), 2);
    const LocationConfig& loc3 = config[1].locations[0];
    EXPECT_EQ(loc3.prefix, "/nimages/");
    ASSERT_EQ(loc3.common.index.size(), 3);
    EXPECT_EQ(loc3.common.index[0], "pindex.html");
    EXPECT_EQ(loc3.common.index[1], "pindex.htm");
    EXPECT_EQ(loc3.common.index[2], "pefault.html");
    const LocationConfig& loc4 = config[1].locations[1];
    EXPECT_EQ(loc4.prefix, "/images/");
    ASSERT_EQ(loc4.common.index.size(), 3);
    EXPECT_EQ(loc4.common.index[0], "index.html");
    EXPECT_EQ(loc4.common.index[1], "index.htm");
    EXPECT_EQ(loc4.common.index[2], "default.html");
}

TEST_F(ServerConfigTest, SimpleCheckOfAutoindex) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    root /var/www/html;\n"
        "    server_name example.com;\n"
        "    listen 127.0.0.1:80;\n"
        "    listen [::1]:80;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].serverNames[0], "example.com");
    EXPECT_EQ(config[0].common.root, "/var/www/html");
    EXPECT_EQ(config[0].listen.at("127.0.0.1"), 80);
    EXPECT_EQ(config[0].listen.at("::1"), 80);
    EXPECT_EQ(config[0].common.autoindex, false);
}

TEST_F(ServerConfigTest, ErrorPageInsufficientArguments) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    error_page 404;\n"  // Missing URI
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, ValidErrorPageDirective) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www;\n"
        "    error_page 404 500 /errors/50x.html;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].common.errorPage[404], "/errors/50x.html");
    EXPECT_EQ(config[0].common.errorPage[500], "/errors/50x.html");
}

TEST_F(ServerConfigTest, ErrorPageInvalidStatusCode) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    error_page 200 /ok.html;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, ErrorPageNonNumericCode) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    error_page 4o4 /errors.html;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, ValidAutoindexOn) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www;\n"
        "    autoindex on;\n"
        "}\n"
    );

    EXPECT_TRUE(config[0].common.autoindex);
}

TEST_F(ServerConfigTest, ValidAutoindexOff) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www;\n"
        "    autoindex off;\n"
        "}\n"
    );

    EXPECT_FALSE(config[0].common.autoindex);
}

TEST_F(ServerConfigTest, InvalidAutoindexValue) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    autoindex maybe;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, DefaultAutoindexValue) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www;\n"
        "}\n"
    );

    EXPECT_FALSE(config[0].common.autoindex);
}

TEST_F(ServerConfigTest, ValidPairedCGI) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www;\n"
        "    cgi_ext .php /usr/bin/php-cgi;\n"
        "    cgi_ext .py /usr/bin/python;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].cgi.size(), 2);
    EXPECT_EQ(config[0].cgi[".php"], "/usr/bin/php-cgi");
    EXPECT_EQ(config[0].cgi[".py"], "/usr/bin/python");
}

TEST_F(ServerConfigTest, MismatchedArgumentCounts) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext .php /usr/bin/php-cgi;\n"
            "    cgi_ext .py ;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, MissingCGIPath) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext .php;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, MissingCGIExt) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext /usr/bin/php-cgi;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, InvalidExtensionFormat) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext php /usr/bin/php-cgi;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, NonAbsolutePath) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext .php php-cgi;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, MultipleCGIDirectives) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext .php /usr/bin/php-cgi;\n"
            "    cgi_ext .py;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, DuplicateExtensions) {
    EXPECT_THROW(
        parseConfig(
            "server {\n"
            "    listen 80;\n"
            "    server_name example.com;\n"
            "    root /var/www;\n"
            "    cgi_ext .php /usr/bin/php-cgi;\n"
            "    cgi_ext .php /updated/php-cgi;\n"
            "}\n"
        ),
        std::runtime_error
    );
}

TEST_F(ServerConfigTest, CaseSensitiveExtensions) {
    std::vector<ServerConfig> config = parseConfig(
        "server {\n"
        "    listen 80;\n"
        "    server_name example.com;\n"
        "    root /var/www;\n"
        "    cgi_ext .PHP /usr/bin/php-cgi;\n"
        "    cgi_ext .py /usr/bin/python;\n"
        "}\n"
    );

    EXPECT_EQ(config[0].cgi[".PHP"], "/usr/bin/php-cgi");
    EXPECT_EQ(config[0].cgi[".py"], "/usr/bin/python");
}
