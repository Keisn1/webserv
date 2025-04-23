#ifndef CONFISTRUCTURE_H
#define CONFISTRUCTURE_H

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

typedef struct LocationConfig {
  std::string prefix;
  std::string root;
  std::vector<std::string> methods;
  /* ...others... */
} LocationConfig;

typedef struct ServerConfig {
  std::map<std::string, int> listen;
  std::vector<std::string> serverNames;
  std::string root;
  std::vector<std::string> indexFiles;
  size_t clientMaxBody;
  std::vector<LocationConfig> locations;
  /* ...others... */
} ServerConfig;

#endif // CONFISTRUCTURE_H