#include <ConfigParser.h>

void ConfigParser::_validateFilename() {
    if (_filename.size() <= 5 || _filename.find(".conf") == std::string::npos) {
        throw std::runtime_error("Invalid file name");
    }
}

void ConfigParser::_parseWorkerConnections(const Directive& directive, EventsConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("worker_connections requires exactly one argument");
    }
    config.workerConnections = static_cast<size_t>(std::strtoul(directive.args[0].c_str(), NULL, 10));
    if (config.workerConnections > MAX_WORKER_CONNECTIONS) {
        throw std::runtime_error("worker_connections exceeded the maximum value");
    }
}

void ConfigParser::_parseUse(const Directive& directive, EventsConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("use requires exactly one argument");
    }
    if (directive.args[0] != "select" && directive.args[0] != "poll" && directive.args[0] != "epoll" && directive.args[0] != "kqueue") {
        throw std::runtime_error("Unknown use method: " + directive.name);
    }
    config.kernelMethod = directive.args[0];
}

void ConfigParser::_parseListen(const Directive& directive, ServerConfig& config) {
    if (directive.args.empty()) {
        throw std::runtime_error("Missing argument for listen directive");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        std::string addr = "0.0.0.0";
        std::string port_str = "80";
        size_t colon_pos = directive.args[i].find_last_of(':');
        if (colon_pos != std::string::npos) {
            addr = directive.args[i].substr(0, colon_pos);
            port_str = directive.args[i].substr(colon_pos + 1);
            if (addr.size() >= 2 && addr.front() == '[' && addr.back() == ']') {
                addr = addr.substr(1, addr.size() - 2);
            }
        } else {
            port_str = directive.args[i];
        }
        char* end;
        long port = std::strtol(port_str.c_str(), &end, 10);
        if (*end != '\0' || port < 1 || port > 65535) {
            throw std::runtime_error("Invalid port in listen directive: " + port_str);
        }
        config.listen[addr] = static_cast<int>(port);
    }
}

void ConfigParser::_parseServerNames(const Directive& directive, ServerConfig& config) {
    if (directive.args.empty()) {
        throw std::runtime_error("Missing argument for server_name directive");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        config.serverNames.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseCgiExt(const Directive& directive, ServerConfig& config) {
    if (directive.args.size() != 2) {
        throw std::runtime_error("cgi_ext requires exactly two arguments");
    } else if (directive.args[0][0] != '.') {
        throw std::runtime_error("Invalid cgi_ext argument: " + directive.args[0]);
    } else if (directive.args[1][0] != '/') {
        throw std::runtime_error("Invalid cgi_ext argument: " + directive.args[1]);
    } else if (config.cgi.find(directive.args[0]) != config.cgi.end()) {
        throw std::runtime_error("Duplicate cgi_ext extension: " + directive.args[0]);
    } 
    config.cgi.insert(std::pair<std::string, std::string>(directive.args[0], directive.args[1]));
}

void ConfigParser::_parseRoot(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("Invalid root directive: exactly one argument required");
    }
    config.root = directive.args[0];
}

void ConfigParser::_parseClientMaxBodySize(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("client_max_body_size requires exactly one argument");
    }
    const std::string& arg = directive.args[0];
    char* end;
    unsigned long value = std::strtoul(arg.c_str(), &end, 10);
    if (end == arg.c_str()) {
        throw std::runtime_error("Invalid client_max_body_size value: " + arg);
    }
    char unit = *end;
    if (unit != '\0') {
        if (end + 1 != arg.c_str() + arg.size()) {
            throw std::runtime_error("Invalid unit in client_max_body_size: " + std::string(end));
        }
        unit = tolower(unit);
        if (unit == 'k') value *= 1024;
        else if (unit == 'm') value *= 1024 * 1024;
        else if (unit == 'g') value *= 1024 * 1024 * 1024;
        else throw std::runtime_error("Invalid unit in client_max_body_size: " + std::string(1, unit));
    }
    config.clientMaxBody = static_cast<size_t>(value);
}

void ConfigParser::_parseAllowMethods(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() == 0) {
        throw std::runtime_error("allow_methods requires more than 0 arguments");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        if (directive.args[i] != "GET" && directive.args[i] != "POST" && directive.args[i] != "DELETE" \
            && directive.args[i] != "HEAD" && directive.args[i] != "PUT" && directive.args[i] != "OPTIONS") {
            throw std::runtime_error("invalid http method: " + directive.args[i]);
        }
        config.allowMethods.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseIndex(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() == 0) {
        throw std::runtime_error("index requires more than 0 arguments");
    }
    for (size_t i = 0; i < directive.args.size(); ++i) {
        config.index.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseErrorPage(const Directive& directive, CommonConfig& config) {
    const std::vector<std::string>& args = directive.args;
    const size_t argCount = args.size();

    if (argCount < 2) {
        throw std::runtime_error("error_page directive requires at least 2 arguments");
    }

    const std::string& uri = args.back();
    if (uri.empty() || (uri[0] != '/' && uri.find("://") == std::string::npos)) {
        throw std::runtime_error("Invalid error_page URI: " + uri);
    }

    for (size_t i = 0; i < argCount - 1; ++i) {
        const std::string& code_str = args[i];
        const char* start = code_str.c_str();
        char* end;
        long code = std::strtoul(start, &end, 10);

        if (*end != '\0' || code_str.empty()) {
            throw std::runtime_error("Invalid error_page code format: " + args[i]);
        } else if (code < 300 || code > 599) {
            throw std::runtime_error("Invalid error_page code at line " + args[i]);
        } else if (config.errorPage.find(code) != config.errorPage.end()) {
            throw std::runtime_error("Duplicate error_page code: " + args[i]);
        }
        config.errorPage[static_cast<int>(code)] = uri;
    }
}

void ConfigParser::_parseAutoindex(const Directive& directive, CommonConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("autoindex requires exactly one argument");
    } if (directive.args[0] == "on") {
        config.autoindex = true;
    } else if (directive.args[0] != "off") {
        throw std::runtime_error("Invalid autoindex argument: " + directive.args[0]);
    }
}
