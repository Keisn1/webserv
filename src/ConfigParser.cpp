#include <ConfigParser.h>

ConfigParser::ConfigParser(const std::string& filename) {
    _filename = filename;
    _validateFilename();
    _makeAst();
    _makeConfig();
};

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

void ConfigParser::_makeAst() {
    std::ifstream configFile(_filename);
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

    for (size_t i = 0; i < locationContext.parameters.size(); ++i) {
        location.prefix.append(locationContext.parameters[i]);
    }

    for (std::vector<Directive>::const_iterator it = locationContext.directives.begin(); it != locationContext.directives.end(); ++it) {
        if (it->name == "root") {
            _parseRoot(*it, location.common);
        } else if (it->name == "client_max_body_size") {
            _parseClientMaxBodySize(*it, location.common);
        } else if (it->name == "allow_methods") {
            _parseAllowMethods(*it, location.common);
        } else if (it->name == "index") {
            _parseIndex(*it, location.common);
        } else {
            throw std::runtime_error("Unknown directive in location context: " + it->name);
        }
        // autoindex
        // cgi_path
        // cgi_ext
        // maybe more
    }
    return location;
}

void ConfigParser::_processServerDirectives(const Context& context, ServerConfig& serverConfig) {
    for (std::vector<Directive>::const_iterator it = context.directives.begin(); it != context.directives.end(); ++it) {
        if (it->name == "listen") {
            _parseListen(*it, serverConfig);
        } else if (it->name == "server_name") {
            _parseServerNames(*it, serverConfig);
        } else if (it->name == "root") {
            _parseRoot(*it, serverConfig.common);
        } else if (it->name == "client_max_body_size") {
            _parseClientMaxBodySize(*it, serverConfig.common);
        } else if (it->name == "allow_methods") {
            _parseAllowMethods(*it, serverConfig.common);
        } else if (it->name == "index") {
            _parseIndex(*it, serverConfig.common);
        } else {
            throw std::runtime_error("Unknown directive in server context: " + it->name);
        }
        // error_page
        // and more
    }
}

void ConfigParser::_parseServerContext(const Context& serverContext) {
    ServerConfig config;
    
    if (!serverContext.parameters.empty()) {
        throw std::runtime_error("Server context doesn't accept parameters");
    }
    _processServerDirectives(serverContext, config);
    for (std::vector<Context>::const_iterator it = serverContext.children.begin(); it != serverContext.children.end(); ++it) {
        if (it->name == "location") {
            config.locations.push_back(_parseLocationContext(*it));
        }
    }

    _serversConfig.push_back(config);
}

void ConfigParser::_parseEventsContext(const Context& eventsContext) {
    EventsConfig eventsConfig;
    eventsConfig.maxEvents = DEFAULT_WORKER_CONNECTIONS;
    eventsConfig.kernelMethod = DEFAULT_USE;
    for (std::vector<Directive>::const_iterator it = eventsContext.directives.begin(); it != eventsContext.directives.end(); ++it) {
        if (it->name == "worker_connections") {
            _parseWorkerConnections(*it, eventsConfig);
        } else if (it->name == "use") {
            _parseUse(*it, eventsConfig);
        } else {
            throw std::runtime_error("Unknown directive in events context: " + it->name);
        }
    }
    _eventsConfig = eventsConfig;
}

bool findDirective(const Context& context, const std::string& identifierKey) {
    for (std::vector<Directive>::const_iterator it = context.directives.begin(); it != context.directives.end(); ++it) {
        if (it->name == identifierKey) {
            return true;
        }
    }
    return false;
}

void ConfigParser::_validateServerContext(const Context& context) {
    if (context.name != "server") {
        throw std::runtime_error("Unexpected context type: " + context.name);
    } else if (!findDirective(context, "listen")) {
        throw std::runtime_error("Server block missing required listen directive");
    } else if (!findDirective(context, "server_name")) {
        throw std::runtime_error("Server block missing required server_name directive");
    } else if (!findDirective(context, "root")) {
        throw std::runtime_error("Server block missing required root directive");
    }
}

void ConfigParser::_makeConfig() {
    for (std::vector<Context>::const_iterator it = _ast.children.begin(); it != _ast.children.end(); ++it) {
        if (it->name == "server") {
            _validateServerContext(*it);
            _parseServerContext(*it);
        } else if (it->name == "events" && _eventsConfig.kernelMethod.empty()) {
            _parseEventsContext(*it);
        } else {
            throw std::runtime_error("Invalid context block: " + it->name);
        }
    }
}

Context ConfigParser::getAst() {
    return _ast;
}

EventsConfig ConfigParser::getEventsConfig() {
    return _eventsConfig;
}

std::vector<ServerConfig> ConfigParser::getServersConfig() {
    return _serversConfig;
}
