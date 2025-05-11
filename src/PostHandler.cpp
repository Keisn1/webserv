#include "PostHandler.h"

PostHandler::PostHandler() {}
PostHandler::~PostHandler() {}

std::map<std::string, std::string> PostHandler::mimeTypes = std::map<std::string, std::string>();
struct MimeInitializer {
    MimeInitializer() {
        PostHandler::mimeTypes[".html"] = "text/html";
        PostHandler::mimeTypes[".htm"] = "text/html";
        PostHandler::mimeTypes[".txt"] = "text/plain";
        PostHandler::mimeTypes[".css"] = "text/css";
        PostHandler::mimeTypes[".jpg"] = "image/jpeg";
        PostHandler::mimeTypes[".jpeg"] = "image/jpeg";
        PostHandler::mimeTypes[".png"] = "image/png";
        PostHandler::mimeTypes[".gif"] = "image/gif";
        PostHandler::mimeTypes[".pdf"] = "application/pdf";
    }
};
static MimeInitializer mimeInit;

bool PostHandler::_checkChunk(Connection* conn) {
    const std::string& body = conn->body;
    size_t i = 0;

    while (i < body.size()) {
        // 1) Locate end of chunk-size line
        size_t line_end = body.find("\r\n", i);
        if (line_end == std::string::npos)
            return false;       // missing CRLF after size

        // 2) Extract hex size string and parse it
        std::string size_str = body.substr(i, line_end - i);
        char* endptr = NULL;
        long length = strtol(size_str.c_str(), &endptr, 16);
        if (endptr == size_str.c_str() || length < 0)
            return false;       // invalid hex number

        // Advance i past the size line "xxxx\r\n"
        i = line_end + 2;

        // 3) If this is the zero-length (terminating) chunk:
        if (length == 0) {
            // Must be followed by final "\r\n"
            if (i + 2 <= body.size() && body.substr(i, 2) == "\r\n")
                return true;        // valid end
            else
                return false;       // missing final CRLF
        }

        // 4) Make sure we have at least 'length' bytes + "\r\n"
        if (i + (size_t)length + 2 > body.size())
            return false;

        // 5) Verify the trailing CRLF after the data
        if (body.substr(i + length, 2) != "\r\n")
            return false;

        // 6) All good: skip past data + CRLF
        i += length + 2;
    }

    // If we exit loop without seeing a zero-length chunk ⇒ malformed
    return false;
}


bool PostHandler::_postValidation(Connection* conn, HttpRequest& request, RouteConfig& config) {
    HttpResponse& resp = conn->_response;

    if (request.uri.empty() || config.root.empty()) {       // uri and root should not empty
        _setErrorResponse(resp, 400, "Bad Request", config);
        return false;
    }
    if (request.uri.find("..") != std::string::npos) {      // not allow /../ appear
        _setErrorResponse(resp, 400, "Bad Request", config);
        return false;
    }
    if (request.headers.find("content-type") == request.headers.end() || request.headers["content-type"] == "") {       // no content-type or no value
        _setErrorResponse(resp, 400, "Bad Request", config);
		return false;
	}
	if (!conn->body.size()) {     // no body
        _setErrorResponse(resp, 400, "Bad Request", config);
		return false;
	}
    if (request.headers.find("transfer-encoding") != request.headers.end()) {
        if (!_checkChunk(conn)) {
            _setErrorResponse(resp, 400, "Bad Request", config);
            return false;       // chunk body is incorrect.
        }
    }
	if (request.headers.find("content-length") != request.headers.end() && atoi(request.headers["content-length"].c_str()) > conn->body.size()) {  // no content-length or content-length bigger than body
		_setErrorResponse(resp, 400, "Bad Request", config);
        return false;
	}
    std::string path = config.root + request.uri;
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {     // Does the exact path exist?
        // Not a file — try the directory part
        std::string dirPath = path.substr(0, path.find_last_of('/'));
        if (stat(dirPath.c_str(), &st) != 0) {        // directory doesn't exist
            _setErrorResponse(resp, 404, "Not Found", config);
            return false;
        }
        if (! S_ISDIR(st.st_mode)) {        // It must be a directory
            _setErrorResponse(resp, 403, "Forbidden", config);
            return false;
        }
        if (access(dirPath.c_str(), R_OK | X_OK) != 0) {        // You need execute (and maybe read) permission on the dir
            _setErrorResponse(resp, 403, "Forbidden", config);
            return false;
        }
    } else {
        // Path exists
        if (!S_ISREG(st.st_mode)) {        // It must be a file
            _setErrorResponse(resp, 403, "Forbidden", config);
            return false;
        }
        if (access(path.c_str(), R_OK) != 0) {        // Check read permission on the file
            _setErrorResponse(resp, 403, "Forbidden", config);
            return false;
        }
        return true;
    }
}

void PostHandler::_writeIntoFile(Connection* conn, HttpRequest& request, RouteConfig& config) {
    std::ofstream file(config.root + request.uri, std::ios::app);
    if (request.headers.find("content-length") != request.headers.end()) {
        for (size_t i = 0; i < conn->body.size(); i++)
        {
            if (conn->body[i] != '\r')
                file << conn->body[i];
        }
    } else if (request.headers.find("transfer-encoding") != request.headers.end()) {
        
        const std::string& body = conn->body;
        size_t i = 0;

        while (i < body.size()) {
            // 1) find the end of the size line
            size_t line_end = body.find("\r\n", i);
            if (line_end == std::string::npos) break;

            // 2) parse hex length
            std::string size_str = body.substr(i, line_end - i);
            char* endptr = NULL;
            long chunk_len = strtol(size_str.c_str(), &endptr, 16);
            if (endptr == size_str.c_str() || chunk_len < 0) break;

            // move index past "<hex>\r\n"
            i = line_end + 2;

            // 3) zero-length chunk = end
            if (chunk_len == 0) {
                break;
            }

            // 4) write exactly chunk_len bytes
            if (i + (size_t)chunk_len > body.size()) break;
            file.write(body.data() + i, chunk_len);

            // 5) advance past data + trailing "\r\n"
            i += chunk_len;
            if (i + 2 > body.size() || body[i] != '\r' || body[i+1] != '\n')
                break;
            i += 2;
        }
    }
}

void PostHandler::_setErrorResponse(HttpResponse& resp, int code, const std::string& message, const RouteConfig& config) {
    std::map<int, std::string>::const_iterator it = config.errorPage.find(code);
    if (it != config.errorPage.end()) {
        std::string errorPagePath = config.root + it->second;
        struct stat fileStat;
        if (stat(errorPagePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            _setResponse(resp, code, message, _getMimeType(errorPagePath), fileStat.st_size, new FileBodyProvider(errorPagePath.c_str()));
            return;
        }
    }
    _setResponse(resp, code, message, "text/plain", message.size(), new StringBodyProvider(message));
}

void PostHandler::_setResponse(HttpResponse& resp, int statusCode, const std::string& statusMessage, const std::string& contentType, size_t contentLength, IBodyProvider* bodyProvider) {
    resp.version = DEFAULT_HTTP_VERSION;
    resp.statusCode = statusCode;
    resp.statusMessage = statusMessage;
    resp.contentType = contentType;
    resp.contentLanguage = DEFAULT_CONTENT_LANGUAGE;
    resp.contentLength = contentLength;
    resp.body = bodyProvider;
}

std::string PostHandler::_getMimeType(const std::string& path) const {
    std::string ext = "";
    std::string::size_type pos = path.rfind('.');
    if (pos != std::string::npos) {
        ext = path.substr(pos);
    }
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return DEFAULT_MIME_TYPE;
}

void PostHandler::handle(Connection* conn, HttpRequest& request, RouteConfig& config) {
    if (conn->preBodySize != conn->body.size()) {
        conn->preBodySize = conn->body.size();
        return;
    }

    HttpResponse& resp = conn->_response;

    // check everything necessary first
    if (!_postValidation(conn, request, config)) {
        conn->setState(Connection::SendResponse);
        return;
    }

    _writeIntoFile(conn, request, config);

    _setResponse(resp, 200, "OK", _getMimeType(request.uri), 0, NULL);
    conn->setState(Connection::SendResponse);
}