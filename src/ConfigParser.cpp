#include <ConfigParser.h>

ConfigParser::ConfigParser() {};
ConfigParser::~ConfigParser() {};
IConfigParser::~IConfigParser() {};

void parseDirectiveOrBlock(TokenStream& ts, Context& ctx) {
    Token nameToken = ts.next();  // e.g., "location"
    std::vector<std::string> tokens;

    // Collect tokens until we hit '{', ';', or '}'
    while (ts.peek().type != PUNCT || (ts.peek().value != "{" && ts.peek().value != ";" && ts.peek().value != "}")) {
        tokens.push_back(ts.next().value);
    }

    if (ts.peek().type == PUNCT && ts.peek().value == "{") {
        // Parse as a block
        ts.expect(PUNCT, "{");
        Context child;
        child.name = nameToken.value;
        child.parameters = tokens;  // e.g., ["/images"]
        while (!ts.accept(PUNCT, "}")) {
            parseDirectiveOrBlock(ts, child);
        }
        ctx.children.push_back(child);
    } else {
        // Parse as a directive
        Directive dir;
        dir.name = nameToken.value;
        dir.args = tokens;
        if (!ts.accept(PUNCT, ";")) {
            throw std::runtime_error("Expected ';' at end of directive");
        }
        ctx.directives.push_back(dir);
    }
}

Context parseBlock(TokenStream& tokenstream) {
    Context context;

    // Read block name
    Token nameToken = tokenstream.next();
    if (nameToken.type != IDENTIFIER) {
        throw std::runtime_error("Expected block name");
    }
    context.name = nameToken.value;

    // Collect parameters until '{'
    while (tokenstream.peek().type != PUNCT || tokenstream.peek().value != "{") {
        Token paramToken = tokenstream.next();
        if (paramToken.type == END_OF_FILE) {
            throw std::runtime_error("Unexpected end of file");
        }
        if (paramToken.type == PUNCT && paramToken.value == ";") {
            throw std::runtime_error("Unexpected ';' in block parameters");
        }
        context.parameters.push_back(paramToken.value);
    }
    tokenstream.expect(PUNCT, "{");

    // Parse contents
    while (!tokenstream.accept(PUNCT, "}")) {
        parseDirectiveOrBlock(tokenstream, context);
    }

    return context;
}

void ConfigParser::_makeAst(const std::string& filename) {
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open configuration file");
    }
    std::string buffer;
    std::string line;
    while (getline(configFile, line)) {
        buffer += line + "\n";
    }
    configFile.close();

    TokenStream tokenstream(buffer);
    _ast = Context();
    _ast.name = "root";

    while (tokenstream.hasMore()) {
        if (tokenstream.accept(PUNCT, ";")) {
            continue;
        }
        if (tokenstream.peek().type == IDENTIFIER && tokenstream.peek(1).value == "{") {
            Context block = parseBlock(tokenstream);
            _ast.children.push_back(block);
        } else {
            const Token& t = tokenstream.peek();
            throw std::runtime_error(
                "Unexpected token at " + toString(t.line) + ":" + toString(t.column) +
                " — expected block name"
            );
        }
    }
}

Context ConfigParser::getAst(const std::string& filename) {
    _makeAst(filename);
    return _ast;
}

ServerConfig ConfigParser::getServerConfig(const std::string& filename) {
    _makeAst(filename);
    return _serverConfig;
}


