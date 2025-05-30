#include <ConfigParser.h>
#include <fstream>
#include <stdexcept>

ConfigParser::ConfigParser(const std::string& filename) {
    _filename = filename;
    _validateFilename();
    _makeAst();
    _makeConfig();
};

ConfigParser::~ConfigParser() {};
IConfigParser::~IConfigParser() {};

void ConfigParser::_parseDirectiveOrBlock(TokenStream& tokenStream, Context& currentBlock) {
    if (!tokenStream.hasMore()) {
        return;
    }
    Token nameToken = tokenStream.next();
    if (nameToken.type != IDENTIFIER) {
        throw std::runtime_error("Expected identifier for directive/block");
    }
    std::set< std::string > terminators;
    terminators.insert("{");
    terminators.insert(";");
    terminators.insert("}");

    std::vector< std::string > invalidArgs;
    invalidArgs.push_back("server");
    invalidArgs.push_back("location");
    invalidArgs.push_back("listen");
    invalidArgs.push_back("server_name");
    invalidArgs.push_back("cgi_ext");
    invalidArgs.push_back("root");
    invalidArgs.push_back("allow_methods");
    invalidArgs.push_back("index");
    invalidArgs.push_back("client_max_body");
    invalidArgs.push_back("error_page");
    invalidArgs.push_back("autoindex");
    invalidArgs.push_back("worker_connections");
    invalidArgs.push_back("use");

    std::vector< std::string > args = tokenStream.collectArguments(terminators, invalidArgs);
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
            _parseDirectiveOrBlock(tokenStream, child);
        }
        currentBlock.children.push_back(child);
    } else if (punctToken.value == ";") {
        tokenStream.next();
        Directive dir;
        dir.name = nameToken.value;
        dir.args = args;
        currentBlock.directives.push_back(dir);
    } else {
        throw std::runtime_error("Expected '{' or ';' after parameters");
    }
}

LocationConfig ConfigParser::_parseLocationContext(const Context& locationContext) {
    LocationConfig locationConfig;
    if (locationContext.parameters.empty()) {
        throw std::runtime_error("Location block missing path parameter");
    }
    for (size_t i = 0; i < locationContext.parameters.size(); ++i) {
        locationConfig.prefix.append(locationContext.parameters[i]);
    }
    for (std::vector< Directive >::const_iterator it = locationContext.directives.begin();
         it != locationContext.directives.end(); ++it) {
        if (it->name == "root") {
            _parseRoot(*it, locationConfig.common);
        } else if (it->name == "client_max_body_size") {
            _parseClientMaxBodySize(*it, locationConfig.common);
        } else if (it->name == "allow_methods") {
            _parseAllowMethods(*it, locationConfig.common);
        } else if (it->name == "index") {
            _parseIndex(*it, locationConfig.common);
        } else if (it->name == "autoindex") {
            _parseAutoindex(*it, locationConfig.common);
        } else if (it->name == "error_page") {
            _parseErrorPage(*it, locationConfig.common);
        } else if (it->name == "cgi_ext") {
            _parseCgiExt(*it, locationConfig);
        } else if (it->name == "return") {
            _parseRedirection(*it, locationConfig);
        } else {
            throw std::runtime_error("Unknown directive in location context: " + it->name);
        }
    }
    return locationConfig;
}

void ConfigParser::_processServerDirectives(const Context& context, ServerConfig& serverConfig) {
    for (std::vector< Directive >::const_iterator it = context.directives.begin(); it != context.directives.end();
         ++it) {
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
        } else if (it->name == "autoindex") {
            _parseAutoindex(*it, serverConfig.common);
        } else if (it->name == "error_page") {
            _parseErrorPage(*it, serverConfig.common);
        } else {
            throw std::runtime_error("Unknown directive in server context: " + it->name);
        }
    }
}

void ConfigParser::_parseServerContext(const Context& serverContext) {
    ServerConfig config;
    if (!serverContext.parameters.empty()) {
        throw std::runtime_error("Server context doesn't accept parameters");
    }
    _processServerDirectives(serverContext, config);
    if (config.serverNames.empty()) {
        config.serverNames.push_back("");
    }
    for (std::vector< Context >::const_iterator it = serverContext.children.begin(); it != serverContext.children.end();
         ++it) {
        if (it->name == "location") {
            config.locations.push_back(_parseLocationContext(*it));
        }
    }
    _serversConfig.push_back(config);
}

void ConfigParser::_parseEventsContext(const Context& eventsContext) {
    EventsConfig eventsConfig;
    eventsConfig.workerConnections = DEFAULT_WORKER_CONNECTIONS;
    eventsConfig.kernelMethod = DEFAULT_USE;
    for (std::vector< Directive >::const_iterator it = eventsContext.directives.begin();
         it != eventsContext.directives.end(); ++it) {
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

bool ConfigParser::_findDirective(const Context& context, const std::string& identifierKey) {
    for (std::vector< Directive >::const_iterator it = context.directives.begin(); it != context.directives.end();
         ++it) {
        if (it->name == identifierKey) {
            return true;
        }
    }
    return false;
}

void ConfigParser::_validateServerContext(const Context& context) {
    if (context.name != "server") {
        throw std::runtime_error("Unexpected context type: " + context.name);
    } else if (!_findDirective(context, "listen")) {
        throw std::runtime_error("Server block missing required listen directive");
    } else if (!_findDirective(context, "root")) {
        throw std::runtime_error("Server block missing required root directive");
    }
}

void ConfigParser::_makeAst() {
    std::ifstream configFile(_filename.c_str());
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
        _parseDirectiveOrBlock(tokenstream, _ast);
    }
}

void ConfigParser::_makeConfig() {
    bool firstEventContext = true;
    for (std::vector< Context >::const_iterator it = _ast.children.begin(); it != _ast.children.end(); ++it) {
        if (it->name == "server") {
            _validateServerContext(*it);
            _parseServerContext(*it);
        } else if (it->name == "events" && firstEventContext) {
            firstEventContext = false;
            _parseEventsContext(*it);
        } else {
            throw std::runtime_error("Invalid context block: " + it->name);
        }
    }
}

Context ConfigParser::getAst() { return _ast; }

EventsConfig ConfigParser::getEventsConfig() { return _eventsConfig; }

std::vector< ServerConfig > ConfigParser::getServersConfig() { return _serversConfig; }
