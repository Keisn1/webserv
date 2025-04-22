#include <ConfigParser.h>

ConfigParser::ConfigParser() {};
ConfigParser::~ConfigParser() {};
IConfigParser::~IConfigParser() {};

void parseDirectiveOrBlock(TokenStream& tokenStream, Context& currentBlock) {
    if (!tokenStream.hasMore()) {
        return;
    }

    Token nameToken = tokenStream.next();
    if (nameToken.type != IDENTIFIER) {
        throw std::runtime_error("Expected identifier for directive/block");
    }

    std::set<std::string> terminators;
    terminators.insert("{");
    terminators.insert(";");
    terminators.insert("}");
    std::vector<std::string> args = tokenStream.collectArguments(terminators);

    if (!tokenStream.hasMore()) {
        throw std::runtime_error("Unexpected end of file");
    }

    Token punctToken = tokenStream.peek();
    if (punctToken.value == "{") {
        tokenStream.expect(PUNCT, "{");
        Context child;
        child.name = nameToken.value;
        child.parameters = args;
        while (!tokenStream.accept(PUNCT, "}")) {
            if (!tokenStream.hasMore()) {
                throw std::runtime_error("Unclosed block");
            }
            parseDirectiveOrBlock(tokenStream, child);
        }
        currentBlock.children.push_back(child);
    } else if (punctToken.value == ";") {
        tokenStream.next(); // Consume ';'
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

LocationConfig ConfigParser::_parseLocationContext(const Context& locationContext) {
    LocationConfig location;
    
    if (locationContext.parameters.empty()) {
        throw std::runtime_error("Location block missing path parameter");
    }
    location.prefix = locationContext.parameters[0]; // magic number

    for (std::vector<Directive>::const_iterator it = locationContext.directives.begin();
         it != locationContext.directives.end(); ++it) {
        if (it->name == "root") {
            if (it->args.size() != 1) {
                throw std::runtime_error("Invalid root directive");
            }
            location.root = it->args[0];
        } else if (it->name == "allow_methods") {
            location.methods = it->args;
        }
        // index
        // autoindex
        // root
        // cgi_path
        // cgi_ext
        // maybe more
    }

    return location;
}

void ConfigParser::_processDirectives(const Context& context, ServerConfig& config) {
    for (std::vector<Directive>::const_iterator it = context.directives.begin(); it != context.directives.end(); ++it) {
        
        if (it->name == "listen") {
            if (it->args.empty()) {
                throw std::runtime_error("Missing port for listen");
            }
            for (size_t i = 0; i < it->args.size(); ++i) {
                std::string addr = i == 0 ? "0.0.0.0" : it->args[i-1];
                int port = atoi(it->args[i].c_str());
                config.listen[addr] = port;
            }
        } else if (it->name == "server_name") {
            config.serverNames = it->args;
        } else if (it->name == "root") {
            if (it->args.size() != 1) {
                throw std::runtime_error("Invalid root directive");
            }
            config.root = it->args[0];
        } else if (it->name == "client_max_body_size") {
            std::stringstream ss(it->args[0]);
            ss >> config.clientMaxBody;
        }
        // allow_methods
        // index
        // error_page
        // and more
    }
}

void ConfigParser::_parseServerContext(const Context& serverContext) {
    ServerConfig config;
    
    if (!serverContext.parameters.empty()) {
        throw std::runtime_error("Server context doesn't accept parameters");
    }

    _processDirectives(serverContext, config);

    for (std::vector<Context>::const_iterator it = serverContext.children.begin(); it != serverContext.children.end(); ++it) {
        if (it->name == "location") {
            config.locations.push_back(_parseLocationContext(*it));
        }
    }

    _serverConfig = config;
}

void ConfigParser::_validateServerContext(const Context& context) {
    if (context.name != "server") {
        throw std::runtime_error("Unexpected context type: " + context.name);
    }
    
    bool hasListen = false;
    for (std::vector<Directive>::const_iterator it = context.directives.begin(); it != context.directives.end(); ++it) {
        if (it->name == "listen") {
            hasListen = true;
        }
    }
    
    // server_name
    // root
    // maybe client_max_body_size
    // more if needed

    if (!hasListen) {
        throw std::runtime_error("Server block missing required listen directive");
    }
}

void ConfigParser::_makeServerConfig() {
    for (std::vector<Context>::const_iterator it = _ast.children.begin();
         it != _ast.children.end(); ++it) {
        if (it->name == "server") {
            _validateServerContext(*it);
            _parseServerContext(*it);
        }
    }
}

Context ConfigParser::getAst(const std::string& filename) {
    _makeAst(filename);
    return _ast;
}

ServerConfig ConfigParser::getServerConfig(const std::string& filename) {
    _makeAst(filename);
    _makeServerConfig();
    return _serverConfig;
}
