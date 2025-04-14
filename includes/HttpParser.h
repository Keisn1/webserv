#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include "IHttpParser.h"

class HttpParser : public IHttpParser {
    void parse(char* buf);
    int error(void) = 0;
	int ready(void) = 0;
    HttpRequest getRequest(void);
};

#endif // HTTPPARSER_H
