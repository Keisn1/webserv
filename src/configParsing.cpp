#include <ConfigParser.h>

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
        config.indexFiles.push_back(directive.args[i]);
    }
}

void ConfigParser::_parseWorkerConnections(const Directive& directive, EventsConfig& config) {
    if (directive.args.size() != 1) {
        throw std::runtime_error("worker_connections requires exactly one argument");
    }
    config.maxEvents = static_cast<size_t>(std::strtoul(directive.args[0].c_str(), NULL, 10));
    if (config.maxEvents > MAX_WORKER_CONNECTIONS) {
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

// index
// autoindex
// error_page
