#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <iostream>
#include <vector>
#include <map>

// first build AST

typedef struct Directive {
  std::string name;                   // "listen", "root", "error_page"
  std::vector<std::string> args;      // { "80" }  or  { "/var/www", "index.html" }
} Directive;

typedef struct Context {
  std::string name;                   // e.g.  "server" or "location"
  std::vector<Directive> directives;  // all directives directly inside this block
  std::vector<Context> children;      // nested blocks (e.g. location { … })
} Context;

// convert tree into typed config structure

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

#endif // CONFIGPARSER_H