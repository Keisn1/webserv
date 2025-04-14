#ifndef IHTTPPARSER_H
#define IHTTPPARSER_H
#include <HttpRequest.h>

class IHttpParser {
  public:
    virtual ~IHttpParser();
    virtual void feed(const char *buf, size_t bufLen) = 0;
    virtual int error(void) = 0;
    virtual int ready(void) = 0;
    virtual HttpRequest getRequest(void) = 0;
};

#endif // IHTTPPARSER_H
