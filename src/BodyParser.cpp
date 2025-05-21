#include "BodyParser.h"
#include "Connection.h"
#include "handlerUtils.h"
#include <sstream>

bool BodyParser::_checkContentLength(Connection* conn, BodyContext& bodyCtx) {
    if (conn->_request.headers.find("content-length") == conn->_request.headers.end()) {
        conn->_bodyFinished = true;
        return false;
    }

    std::stringstream ss(conn->_request.headers["content-length"]);
    ss >> bodyCtx.contentLength;
    if (bodyCtx.contentLength == 0) {
        conn->_bodyFinished = true;
        return false;
    }

    if (bodyCtx.contentLength > conn->route.cfg.clientMaxBody) {
        setErrorResponse(conn->_response, 413, "Content Too Large", conn->route.cfg);
        conn->setState(Connection::SendResponse);
        return false;
    }
    return true;
}

void BodyParser::_parseContentLength(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;
    Buffer& readBuf = conn->_readBuf;

    if (bodyCtx.contentLength == 0) {
        if (!_checkContentLength(conn, bodyCtx))
            return;
    }
    if ((bodyCtx.bytesReceived + readBuf.size()) < bodyCtx.contentLength) {
        conn->_tempBody = std::string(conn->_readBuf.data(), conn->_readBuf.size());
        bodyCtx.bytesReceived += conn->_tempBody.size();
        readBuf.clear();
    } else {
        int countRest = bodyCtx.contentLength - bodyCtx.bytesReceived;
        conn->_tempBody = std::string(readBuf.data(), bodyCtx.contentLength - bodyCtx.bytesReceived);
        bodyCtx.bytesReceived = bodyCtx.contentLength;
        conn->_bodyFinished = true;
        readBuf.advance(countRest);
    }
}

void BodyParser::parse(Connection* conn) {
    if (conn->_request.headers.find("transfer-encoding") != conn->_request.headers.end()) {
        if (!conn->_readBuf.size()) {
            conn->_bodyFinished = true;
            return;
        }
        size_t subContentLength;
        std::string subContent;
        std::string body = std::string(conn->_readBuf.data());
        if (body.find("0\r\n\r\n") == std::string::npos) {
            setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
            conn->setState(Connection::SendResponse);
            return;
    }
        size_t pos = 0;
        size_t holdPos = 0;
        while(pos < body.size()) {
            holdPos = pos;
            pos = body.find("\r\n", holdPos);
            std::istringstream iss(body.substr(holdPos, pos - holdPos));
            iss >> subContentLength;
            pos += 2;
            holdPos = pos;
            pos = body.find("\r\n", holdPos);
            subContent = body.substr(holdPos, pos - holdPos);
            if (subContent.size() != subContentLength) {
                setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
                conn->setState(Connection::SendResponse);
                return;
            }
            conn->_tempBody += subContent;
            pos += 2;
        }
        conn->_bodyFinished = true;
        return;
    }
    _parseContentLength(conn);
}