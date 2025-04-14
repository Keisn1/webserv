#ifndef IHTTPPARSER_H
#define IHTTPPARSER_H
#include <HttpRequest.h>

class IHttpParser {
  public:
    virtual ~IHttpParser();
    virtual void
    parse(char *buf) = 0; // !!! buffer needs to be NULL-terminated !!!
    virtual int error(void) = 0;
    virtual int ready(void) = 0;
    virtual HttpRequest getRequest(void) = 0;
};

#endif // IHTTPPARSER_H
