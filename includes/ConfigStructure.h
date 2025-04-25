#ifndef CONFIGSTRUCTURE_H
#define CONFIGSTRUCTURE_H

#include <map>
#include <vector>
#include <iostream>

#define DEFAULT_WORKER_CONNECTIONS 512
#define MAX_WORKER_CONNECTIONS 1024
#define DEFAULT_USE "epoll"

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

  EventsConfig() :
    workerConnections(DEFAULT_WORKER_CONNECTIONS),
    kernelMethod(DEFAULT_USE)
    {}
} EventsConfig;

typedef struct CommonConfig {
  std::string root;
  std::vector<std::string> allowMethods;
  std::vector<std::string> index;
  size_t clientMaxBody;
  std::map<int, std::string> errorPage;
  bool autoindex;

  CommonConfig() : 
    root(),
    allowMethods(),
    index(),
    clientMaxBody(0),
    errorPage(),
    autoindex(false)
    {}
} CommonConfig;

typedef struct LocationConfig {
  std::string prefix;
  CommonConfig common;
} LocationConfig;

typedef struct ServerConfig {
  std::map<std::string, int> listen;
  std::vector<std::string> serverNames;
  // cgi_path
  // cgi_ext
  CommonConfig common;
  std::vector<LocationConfig> locations;
} ServerConfig;

#endif // CONFIGSTRUCTURE_H