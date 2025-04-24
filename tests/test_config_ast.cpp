#include <ConfigParser.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

Context buildAST(const std::string& config) {
    ConfigParser parser(config);
    return parser.getAst();
}

TEST(ASTTest, ParsesSingleServerBlock) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    root /var/www/html;\n"
         << "    server_name example.com;\n"
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    EXPECT_EQ(server.name, "server");
    ASSERT_EQ(server.directives.size(), 3);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[0].args.size(), 1);
    EXPECT_EQ(server.directives[0].args[0], "80");
    EXPECT_EQ(server.directives[1].name, "root");
    EXPECT_EQ(server.directives[1].args[0], "/var/www/html");
    EXPECT_EQ(server.directives[2].name, "server_name");
    EXPECT_EQ(server.directives[2].args[0], "example.com");
    EXPECT_TRUE(server.children.empty());
}

TEST(ASTTest, ParsesNestedLocation) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    root /var/www/html;\n"
         << "    server_name example.com;\n"
         << "    location /images/ {\n"
         << "        root /var/www/images;\n"
         << "    }\n"
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    ASSERT_EQ(server.name, "server");
    ASSERT_EQ(server.directives.size(), 3);
    EXPECT_EQ(server.directives[0].name, "listen");
    ASSERT_EQ(server.children.size(), 1);
    const Context& loc = server.children[0];
    EXPECT_EQ(loc.name, "location");
    ASSERT_EQ(loc.directives.size(), 1);
    EXPECT_EQ(loc.directives[0].name, "root");
    EXPECT_EQ(loc.directives[0].args[0], "/var/www/images");
}

TEST(ASTTest, IgnoresCommentsAndWhitespace) {
    std::ofstream file;
    file.open("configTest");
    file << "# Global comment\n"
         << "server {  # inline comment\n"
         << "    listen    3000;   \n"
         << "    # another comment\n"
         << "    root    \"/srv/www\";\n"
         << "    server_name example.com;\n" 
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    ASSERT_EQ(server.directives.size(), 3);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[0].args[0], "3000");
    EXPECT_EQ(server.directives[1].name, "root");
    EXPECT_EQ(server.directives[1].args[0], "/srv/www");
    EXPECT_EQ(server.directives[2].name, "server_name");
    EXPECT_EQ(server.directives[2].args[0], "example.com");
}

TEST(ASTTest, ThrowsOnMissingListen) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    root /var/www/html;\n"
         << "    server_name example.com;\n"
         << "}\n";
    file.close();
    EXPECT_THROW({
        Context root = buildAST("configTest");
    }, std::runtime_error);
    std::remove("configTest");
}

TEST(ASTTest, ThrowsOnMissingRoot) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    server_name example.com;\n"
         << "}\n";
    file.close();
    EXPECT_THROW({
        Context confiroot = buildAST("configTest");
    }, std::runtime_error);
    std::remove("configTest");
}

TEST(ASTTest, ThrowsOnMissingServerName) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    root /var/www/html;\n"
         << "}\n";
    file.close();
    EXPECT_THROW({
        Context confiroot = buildAST("configTest");
    }, std::runtime_error);
    std::remove("configTest");
}

TEST(ASTTest, ThrowsOnEmptyBlock) {
    std::ofstream file;
    file.open("configTest");
    file << "server {}\n";
    file.close();
    EXPECT_THROW({
        Context confiroot = buildAST("configTest");
    }, std::runtime_error);
    std::remove("configTest");
}

TEST(ASTTest, ThrowsOnMissingSemicolon) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80\n"
         << "    root /var/www/html;\n"
         << "    server_name example.com;\n"
         << "}\n";
    file.close();
    EXPECT_THROW({
        Context root = buildAST("configTest");
    }, std::runtime_error);
    std::remove("configTest");
}

TEST(ASTTest, ThrowsOnUnclosedBlock) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    root /var/www/html;\n"
         << "    server_name example.com;\n";
    file.close();
    EXPECT_THROW({
        Context root = buildAST("configTest");
    }, std::runtime_error);
    std::remove("configTest");
}

TEST(ASTTest, ParsesMultipleTopLevelBlocks) {
    std::ofstream file;
    file.open("configTest");
    file << "server { listen 80; root /var/www; server_name example.com; }\n"
         << "server { listen 8080; root /var/www; server_name test.com; }\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 2);
    EXPECT_EQ(root.children[0].directives[0].args[0], "80");
    EXPECT_EQ(root.children[1].directives[0].args[0], "8080");
}

TEST(ASTTest, HandlesMixedDirectivesAndBlocks) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    server_name example.com;\n"
         << "    location / {\n"
         << "        index index.html;\n"
         << "    }\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    const Context& server = root.children[0];
    ASSERT_EQ(server.directives.size(), 3);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[1].name, "server_name");
    EXPECT_EQ(server.directives[2].name, "root");
    ASSERT_EQ(server.children.size(), 1);
    EXPECT_EQ(server.children[0].directives[0].name, "index");
}

TEST(ASTTest, ParsesQuotedArguments) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    server_name example.com;\n"
         << "    root \"/var/www/my site\";\n"
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    const Context& server = root.children[0];
    ASSERT_EQ(server.directives.size(), 3);
    EXPECT_EQ(server.directives[2].args[0], "/var/www/my site");
}

TEST(ASTTest, ThrowsOnNonexistentFile) {
    EXPECT_THROW({
        Context root = buildAST("nonexistent.conf");
    }, std::runtime_error);
}

TEST(ASTTest, HandlesEmptyFile) {
    std::ofstream file;
    file.open("configTest");
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");
    EXPECT_TRUE(root.children.empty());
}

TEST(ASTTest, MultipleServers) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 80;\n"
         << "    root /var/www/html;\n"
         << "    server_name example.com;\n"
         << "}\n"
         << "\n"
         << "server {\n"
         << "    listen 81;\n"
         << "    root /var/www/htmn;\n"
         << "    server_name example2.com;\n"
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 2);
    const Context& server1 = root.children[0];
    EXPECT_EQ(server1.name, "server");
    ASSERT_EQ(server1.directives.size(), 3);
    EXPECT_EQ(server1.directives[0].name, "listen");
    EXPECT_EQ(server1.directives[0].args.size(), 1);
    EXPECT_EQ(server1.directives[0].args[0], "80");
    EXPECT_EQ(server1.directives[1].name, "root");
    EXPECT_EQ(server1.directives[1].args[0], "/var/www/html");
    EXPECT_EQ(server1.directives[2].name, "server_name");
    EXPECT_EQ(server1.directives[2].args[0], "example.com");
    EXPECT_TRUE(server1.children.empty());
    const Context& server2 = root.children[1];
    EXPECT_EQ(server2.name, "server");
    ASSERT_EQ(server2.directives.size(), 3);
    EXPECT_EQ(server2.directives[0].name, "listen");
    EXPECT_EQ(server2.directives[0].args.size(), 1);
    EXPECT_EQ(server2.directives[0].args[0], "81");
    EXPECT_EQ(server2.directives[1].name, "root");
    EXPECT_EQ(server2.directives[1].args[0], "/var/www/htmn");
    EXPECT_EQ(server2.directives[2].name, "server_name");
    EXPECT_EQ(server2.directives[2].args[0], "example2.com");
    EXPECT_TRUE(server2.children.empty());
}