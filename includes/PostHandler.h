#ifndef POSTHANDLER_H
#define POSTHANDLER_H

#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>
#include <stdlib.h>


#include "Router.h"
#include "Connection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#define DEFAULT_MIME_TYPE "application/octet-stream"
#define DEFAULT_HTTP_VERSION "HTTP/1.1"
#define DEFAULT_CONTENT_LANGUAGE "en-US"
#define MAX_PATH_LENGTH 4096

class PostHandler : public IHandler {
	private:
	  bool _postValidation(Connection* conn, const HttpRequest& request, const RouteConfig& config);
	  bool _checkChunk(Connection* conn);
	  std::string _setPath(std::string root, std::string uri);

	  void _writeIntoFile(Connection* conn, const HttpRequest& request, const RouteConfig& config);

	  void _setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider);
	  void _setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config);

	  std::string _getMimeType(const std::string& path) const;
	  
	public:
	  static std::map<std::string, std::string> mimeTypes;
	  PostHandler();
	  ~PostHandler();
	  void handle(Connection* conn, const HttpRequest& req, const RouteConfig& config);
  };

#endif