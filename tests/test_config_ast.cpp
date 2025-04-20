#include <ConfigParser.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;

// Helper to generate AST root from config string
Context buildAST(const string& config) {
    TokenStream ts(config);
    ConfigParser parser;

    parser.makeAst(ts);
    return parser.getAst();
}

TEST(ASTTest, ParsesSingleServerBlock) {
    const string input =
        "server {\n"
        "    listen 80;\n"
        "    root /var/www/html;\n"
        "}\n";

    Context root = buildAST(input);
    // Should have one top-level block
    ASSERT_EQ(root.children.size(), 1);

    const Context& server = root.children[0];
    EXPECT_EQ(server.name, "server");
    // Should have two directives inside server
    ASSERT_EQ(server.directives.size(), 2);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[0].args.size(), 1);
    EXPECT_EQ(server.directives[0].args[0], "80");
    EXPECT_EQ(server.directives[1].name, "root");
    EXPECT_EQ(server.directives[1].args[0], "/var/www/html");
    // No nested contexts
    EXPECT_TRUE(server.children.empty());
}

TEST(ASTTest, ParsesNestedLocation) {
    const string input =
        "server {\n"
        "    listen 8080;\n"
        "    location /images/ {\n"
        "        root /var/www/images;\n"
        "    }\n"
        "}\n";

    Context root = buildAST(input);
    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    ASSERT_EQ(server.name, "server");
    // One directive + one child context
    ASSERT_EQ(server.directives.size(), 1);
    EXPECT_EQ(server.directives[0].name, "listen");
    ASSERT_EQ(server.children.size(), 1);

    const Context& loc = server.children[0];
    EXPECT_EQ(loc.name, "location");
    // Location should have one directive
    ASSERT_EQ(loc.directives.size(), 1);
    EXPECT_EQ(loc.directives[0].name, "root");
    EXPECT_EQ(loc.directives[0].args[0], "/var/www/images");
}

TEST(ASTTest, IgnoresCommentsAndWhitespace) {
    const string input =
        "# Global comment\n"
        "server {  # inline comment\n"
        "    listen    3000;   \n"
        "    # another comment\n"
        "    root    \"/srv/www\";\n"
        "}\n";

    Context root = buildAST(input);
    ASSERT_EQ(root.children.size(), 1);
    const Context& server = root.children[0];
    ASSERT_EQ(server.directives.size(), 2);
    EXPECT_EQ(server.directives[0].name, "listen");
    EXPECT_EQ(server.directives[0].args[0], "3000");
    EXPECT_EQ(server.directives[1].name, "root");
    EXPECT_EQ(server.directives[1].args[0], "/srv/www");
}