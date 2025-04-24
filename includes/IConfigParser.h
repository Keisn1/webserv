#ifndef ICONFIGPARSER_H
#define ICONFIGPARSER_H

#include <ConfigStructure.h>

class IConfigParser {
  public:
    virtual ~IConfigParser();
    virtual EventsConfig getEventsConfig() = 0;
    virtual std::vector<ServerConfig> getServersConfig() = 0;
};

#endif // ICONFIGPARSER_H
