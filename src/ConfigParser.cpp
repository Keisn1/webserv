#include <ConfigParser.h>

ConfigParser::ConfigParser() {};
ConfigParser::~ConfigParser() {};
IConfigParser::~IConfigParser() {};

void parseDirectiveOrBlock(TokenStream& ts, Context& currentBlock) {
    if (!ts.hasMore()) return;

    Token nameToken = ts.next();
    if (nameToken.type != IDENTIFIER) {
        throw std::runtime_error("Expected identifier for directive/block");
    }

    std::set<std::string> terminators;
    terminators.insert("{");
    terminators.insert(";");
    terminators.insert("}");
    std::vector<std::string> args = ts.collectArguments(terminators);

    if (!ts.hasMore()) {
        throw std::runtime_error("Unexpected end of file");
    }

    Token punctToken = ts.peek();
    if (punctToken.value == "{") {
        ts.expect(PUNCT, "{");
        Context child;
        child.name = nameToken.value;
        child.parameters = args;
        while (!ts.accept(PUNCT, "}")) {
            if (!ts.hasMore()) {
                throw std::runtime_error("Unclosed block");
            }
            parseDirectiveOrBlock(ts, child);
        }
        currentBlock.children.push_back(child);
    } else if (punctToken.value == ";") {
        ts.next(); // Consume ';'
        Directive dir;
        dir.name = nameToken.value;
        dir.args = args;
        currentBlock.directives.push_back(dir);
    } else {
        throw std::runtime_error("Expected '{' or ';' after parameters");
    }
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
        parseDirectiveOrBlock(tokenstream, _ast);
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
