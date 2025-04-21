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

Context parseBlock(TokenStream& ts) {
    Context ctx;

    // Read block name
    Token nameToken = ts.next();
    if (nameToken.type != IDENTIFIER) {
        throw std::runtime_error("Expected block name");
    }
    ctx.name = nameToken.value;

    // Collect parameters until '{'
    while (ts.peek().type != PUNCT || ts.peek().value != "{") {
        Token paramToken = ts.next();
        if (paramToken.type == END_OF_FILE) {
            throw std::runtime_error("Unexpected end of file");
        }
        if (paramToken.type == PUNCT && paramToken.value == ";") {
            throw std::runtime_error("Unexpected ';' in block parameters");
        }
        ctx.parameters.push_back(paramToken.value);
    }
    ts.expect(PUNCT, "{");

    // Parse contents
    while (!ts.accept(PUNCT, "}")) {
        parseDirectiveOrBlock(ts, ctx);
    }

    return ctx;
}

void ConfigParser::makeAst(TokenStream& ts) {
    _ast = Context();
    _ast.name = "root";

    // While there are tokens before EOF
    while (ts.peek().type != END_OF_FILE) {
        if (ts.accept(PUNCT, ";"))
            continue;
        if (ts.peek().type == IDENTIFIER && ts.peek(1).value == "{") {
            Context block = parseBlock(ts);
            _ast.children.push_back(block);
        } else {
            const Token& t = ts.peek();
            throw std::runtime_error(
                "Unexpected token at " + toString(t.line) + ":" + toString(t.column) +
                " — expected block name"
            );
        }
    }
}

Context ConfigParser::getAst() {
    return _ast;
}

ServerConfig ConfigParser::getServerConfig(const std::string& filename) {
    // READ AND EXTRACT FILE
    std::ifstream configFile(filename);
    std::string line;
    while (getline(configFile, line)) {
        _buffer.append(line);
    }
    configFile.close();

    TokenStream ts(_buffer);

    makeAst(ts);


    return _serverConfig;
}


