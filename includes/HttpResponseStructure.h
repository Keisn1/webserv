#ifndef HTTP_RESPONSE_WRITING_H
#define HTTP_RESPONSE_WRITING_H

#include <string>
#include <map>

struct RawHttpResponse {
  std::string httpVersion;                  	  // "HTTP/1.1"
  int statusCode;                            		// 200, 404
  std::string statusMessage;                    // "OK", "Not Found"
  std::map<std::string, std::string> headers;   // {"Content-Type", "text/html"}
  char* body;                                   // The response content (for later use)
};

struct HttpResponse {
  char* response;
  size_t responseSize;
};

char* serializeResponse(const HttpResponse& response);

#endif