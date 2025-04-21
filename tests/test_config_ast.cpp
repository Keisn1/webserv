#include <ConfigParser.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Helper to generate AST root from config string
Context buildAST(const std::string& config) {
    ConfigParser parser;
    return parser.getAst(config);
}

TEST(ASTTest, ParsesSingleServerBlock) {
    std::ofstream file;
    file.open("configTest");
    file <<  "server {\n"
         << "    listen 80;\n"
         << "    root /var/www/html;\n"
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    EXPECT_EQ(server.name, "server");
    ASSERT_EQ(server.directives.size(), 2);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[0].args.size(), 1);
    EXPECT_EQ(server.directives[0].args[0], "80");
    EXPECT_EQ(server.directives[1].name, "root");
    EXPECT_EQ(server.directives[1].args[0], "/var/www/html");
    EXPECT_TRUE(server.children.empty());
}

TEST(ASTTest, ParsesNestedLocation) {
    std::ofstream file;
    file.open("configTest");
    file << "server {\n"
         << "    listen 8080;\n"
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
    ASSERT_EQ(server.directives.size(), 1);
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
         << "}\n";
    file.close();
    Context root = buildAST("configTest");
    std::remove("configTest");

    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    ASSERT_EQ(server.directives.size(), 2);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[0].args[0], "3000");
    EXPECT_EQ(server.directives[1].name, "root");
    EXPECT_EQ(server.directives[1].args[0], "/srv/www");
}