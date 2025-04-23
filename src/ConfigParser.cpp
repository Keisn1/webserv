#include <ConfigParser.h>

ConfigParser::ConfigParser(const std::string& filename) {
    _makeAst(filename);
    _makeServerConfig();
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

void ConfigParser::_processServerDirectives(const Context& context, ServerConfig& config) {
    for (std::vector<Directive>::const_iterator it = context.directives.begin(); it != context.directives.end(); ++it) {
        if (it->name == "listen") {
            if (it->args.empty()) {
                throw std::runtime_error("Missing argument for listen directive");
            }
        
            for (size_t i = 0; i < it->args.size(); ++i) {
                std::string addr = "0.0.0.0"; // default IP
                std::string port_str = "80";  // default port
                size_t colon_pos = it->args[i].find_last_of(':');
        
                if (colon_pos != std::string::npos) {
                    addr = it->args[i].substr(0, colon_pos);
                    port_str = it->args[i].substr(colon_pos + 1);
        
                    if (addr.size() >= 2 && addr.front() == '[' && addr.back() == ']') {
                        addr = addr.substr(1, addr.size() - 2);
                    }
                } else {
                    port_str = it->args[i];
                }
        
                char* end;
                long port = std::strtol(port_str.c_str(), &end, 10);
                if (*end != '\0' || port < 1 || port > 65535) {
                    throw std::runtime_error("Invalid port in listen directive: " + port_str);
                }
                config.listen[addr] = static_cast<int>(port);
            }
        } else if (it->name == "server_name") {
            config.serverNames = it->args;
        } else if (it->name == "root") {
            if (it->args.size() != 1) {
                throw std::runtime_error("Invalid root directive");
            }
            config.root = it->args[0];
        } else if (it->name == "client_max_body_size") {
            if (it->args.size() != 1) {
                throw std::runtime_error("client_max_body_size requires exactly one argument");
            }
        
            const std::string& arg = it->args[0];
            size_t value;
            char unit = '\0';
            std::size_t pos = 0;
            try {
                value = std::stoull(arg, &pos);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid client_max_body_size value: " + arg);
            }
        
            if (pos < arg.size()) {
                if (arg.size() - pos > 1) {
                    throw std::runtime_error("Invalid unit in client_max_body_size: " + arg);
                }
                unit = std::tolower(arg[pos]);
            }
        
            switch (unit) {
                case 'k': value *= 1024; break;
                case 'm': value *= 1024 * 1024; break;
                case 'g': value *= 1024 * 1024 * 1024; break;
                case '\0': break;
                default:
                    throw std::runtime_error("Invalid unit in client_max_body_size: " + std::string(1, unit));
            }
            config.clientMaxBody = value;
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

    _processServerDirectives(serverContext, config);

    for (std::vector<Context>::const_iterator it = serverContext.children.begin(); it != serverContext.children.end(); ++it) {
        if (it->name == "location") {
            config.locations.push_back(_parseLocationContext(*it));
        }
    }

    _serverConfig = config;
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
    }
    if (!findDirective(context, "listen")) {
        throw std::runtime_error("Server block missing required listen directive");
    }
    if (!findDirective(context, "server_name")) {
        throw std::runtime_error("Server block missing required server_name directive");
    }
    if (!findDirective(context, "root")) {
        throw std::runtime_error("Server block missing required root directive");
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

Context ConfigParser::getAst() {
    return _ast;
}

ServerConfig ConfigParser::getServerConfig() {
    return _serverConfig;
}
