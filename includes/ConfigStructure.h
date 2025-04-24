#ifndef CONFIGSTRUCTURE_H
#define CONFIGSTRUCTURE_H

#include <map>
#include <vector>
#include <iostream>

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
  size_t maxEvents;
  std::string kernelMethod;
} EventsConfig;

typedef struct CommonConfig {
  std::string root;
  std::vector<std::string> allowMethods;
  std::vector<std::string> indexFiles;
  size_t clientMaxBody;
  std::map<int, std::string> errorPages;
  // to add more
} CommonConfig;

typedef struct LocationConfig {
  std::string prefix;
  CommonConfig common;
  // to add more
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