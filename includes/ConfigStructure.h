#ifndef CONFIGSTRUCTURE_H
#define CONFIGSTRUCTURE_H

#include <iostream>
#include <map>
#include <vector>

#define DEFAULT_WORKER_CONNECTIONS 512
#define MAX_WORKER_CONNECTIONS 1024
#define DEFAULT_USE "epoll"
#define KB 1024
#define MB 1024 * 1024
#define GB 1024 * 1024 * 1024

typedef struct Directive {
    std::string name;
    std::vector<std::string> args;
} Directive;

typedef struct Context {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<Directive> directives;
    std::vector<Context> children;
} Context;

typedef struct EventsConfig {
    size_t workerConnections;
    std::string kernelMethod;

    EventsConfig() : workerConnections(DEFAULT_WORKER_CONNECTIONS), kernelMethod(DEFAULT_USE) {}
} EventsConfig;

typedef struct CommonConfig {
    std::string root;
    std::vector<std::string> allowMethods;
    std::vector<std::string> index;
    size_t clientMaxBody;
    std::map<int, std::string> errorPage;
    bool autoindex;

    CommonConfig() : root(), allowMethods(), index(), clientMaxBody(MB), errorPage(), autoindex(false) {}
} CommonConfig;

typedef struct LocationConfig {
    std::string prefix;
    std::map<std::string, std::string> cgi;
    CommonConfig common;
} LocationConfig;

typedef struct ServerConfig {
    std::map<std::string, int> listen;
    std::vector<std::string> serverNames;
    CommonConfig common;
    std::vector<LocationConfig> locations;
} ServerConfig;

#endif // CONFIGSTRUCTURE_H
