#ifndef HANDLERUTILS_H
#define HANDLERUTILS_H

#include <string>
#include <sys/stat.h>
#include <string>
#include <cstdlib>
#include <ctime>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RouteConfig.h"

#define DEFAULT_MIME_TYPE "application/octet-stream"
#define DEFAULT_HTTP_VERSION "HTTP/1.1"
#define DEFAULT_CONTENT_LANGUAGE "en-US"
#define MAX_URI_LENGTH 4096

extern std::map< std::string, std::string > mimeTypes;
extern std::map< int, std::string > statusPhrases;

std::string getMimeType(const std::string& path);
int hexToInt(char c);
std::string decodePercent(const std::string& str);
std::string normalizePath(const std::string& root, const std::string& uri);
std::string extractQuery(const std::string& uri);
void setHeader(HttpResponse& resp, std::string key, std::string value);
void setResponse(HttpResponse& resp, int statusCode, const std::string& contentType,
                 size_t contentLength, IBodyProvider* bodyProvider);
void setErrorResponse(HttpResponse& resp, int code, const RouteConfig& config);
bool validateRequest(HttpResponse& resp, const HttpRequest& req, const RouteConfig& config, std::string& path,
                     struct stat& pathStat);
std::string getSessionID();

#endif // HANDLERUTILS_H
