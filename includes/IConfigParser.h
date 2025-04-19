#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <ConfigStructure.h>

typedef struct ServerConfig;

class IConfigParser {
  public:
    virtual ~IConfigParser();
    virtual ServerConfig getServerConfig(const int fd) = 0;
};

#endif // CONFIGPARSER_H
