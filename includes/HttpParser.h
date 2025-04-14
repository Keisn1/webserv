#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include "IHttpParser.h"

class HttpParser : public IHttpParser {
  public:
    void feed(const char *buf, size_t bufLen) {
        (void)buf;
        (void)bufLen;
    };
    int error(void) { return 0; };
    int ready(void) { return 0; };
    HttpRequest getRequest(void) { return HttpRequest{""}; };
};

#endif // HTTPPARSER_H
