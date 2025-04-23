#ifndef ICONFIGPARSER_H
#define ICONFIGPARSER_H

#include <ConfigStructure.h>

class IConfigParser {
  public:
    virtual ~IConfigParser();
    virtual ServerConfig getServerConfig() = 0;
};

#endif // ICONFIGPARSER_H
