#ifndef ICONFIGPARSER_H
#define ICONFIGPARSER_H

#include <ConfigStructure.h>

class IConfigParser {
  public:
    virtual ~IConfigParser();
    virtual std::vector<ServerConfig> getServersConfig() = 0;
};

#endif // ICONFIGPARSER_H
