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

void BodyParser::_parseTranfsferEncoding(Connection* conn) {
    BodyContext& bodyCtx = conn->bodyCtx;
    if (!conn->_readBuf.size()) {
        conn->_bodyFinished = true;
        return;
    }
    std::string subContent;
    std::string body = bodyCtx.tempStore + std::string(conn->_readBuf.data());
    size_t pos = 0;
    size_t holdPos = 0;
    while(pos < body.size()) {
        if (bodyCtx.lengthOrBody == false) {
            holdPos = pos;
            if ((pos = body.find("\r\n", holdPos)) == std::string::npos) {
                bodyCtx.tempStore = body.substr(holdPos);
                return;
            }
            std::istringstream iss(body.substr(holdPos, pos - holdPos));
            iss >> bodyCtx.contentLength;
            if (bodyCtx.contentLength == 0) {
                if ((pos = body.find("\r\n\r\n")) == std::string::npos) {
                    bodyCtx.tempStore = body.substr(holdPos);
                    return;
                }
                conn->_bodyFinished = true;
                return;
            }
            bodyCtx.lengthOrBody = true;
            pos += 2;
        }
        if (bodyCtx.lengthOrBody == true) {
            holdPos = pos;
            if (holdPos == body.size()) {
                bodyCtx.tempStore = "";
                return;
            }
            if ((pos = body.find("\r\n", holdPos)) == std::string::npos) {
                bodyCtx.tempStore = body.substr(holdPos);
                return;
            }
            subContent = body.substr(holdPos, pos - holdPos);
            if (subContent.size() != bodyCtx.contentLength) {
                setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
                conn->setState(Connection::SendResponse);
                return;
            }
            conn->_tempBody += subContent;
            bodyCtx.lengthOrBody = false;
            pos += 2;
            if (pos == body.size()) {
                bodyCtx.tempStore = "";
            }
            bodyCtx.tempStore = body.substr(pos);
        }
    }
    setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
    conn->setState(Connection::SendResponse);
    return;
}

void BodyParser::parse(Connection* conn) {
    if (conn->_request.headers.find("transfer-encoding") != conn->_request.headers.end()) {
        // BodyContext& bodyCtx = conn->bodyCtx;
        // if (!conn->_readBuf.size()) {
        //     conn->_bodyFinished = true;
        //     return;
        // }
        // std::string subContent;
        // std::string body = bodyCtx.tempStore + std::string(conn->_readBuf.data());
        // size_t pos = 0;
        // size_t holdPos = 0;
        // while(pos < body.size()) {
        //     if (bodyCtx.lengthOrBody == false) {
        //         holdPos = pos;
        //         if ((pos = body.find("\r\n", holdPos)) == std::string::npos) {
        //             bodyCtx.tempStore = body.substr(holdPos);
        //             return;
        //         }
        //         std::istringstream iss(body.substr(holdPos, pos - holdPos));
        //         iss >> bodyCtx.contentLength;
        //         if (bodyCtx.contentLength == 0) {
        //             if ((pos = body.find("\r\n\r\n")) == std::string::npos) {
        //                 bodyCtx.tempStore = body.substr(holdPos);
        //                 return;
        //             }
        //             conn->_bodyFinished = true;
        //             return;
        //         }
        //         bodyCtx.lengthOrBody = true;
        //         pos += 2;
        //     }
        //     if (bodyCtx.lengthOrBody == true) {
        //         holdPos = pos;
        //         if (holdPos == body.size()) {
        //             bodyCtx.tempStore = "";
        //             return;
        //         }
        //         if ((pos = body.find("\r\n", holdPos)) == std::string::npos) {
        //             bodyCtx.tempStore = body.substr(holdPos);
        //             return;
        //         }
        //         subContent = body.substr(holdPos, pos - holdPos);
        //         if (subContent.size() != bodyCtx.contentLength) {
        //             setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
        //             conn->setState(Connection::SendResponse);
        //             return;
        //         }
        //         conn->_tempBody += subContent;
        //         bodyCtx.lengthOrBody = false;
        //         pos += 2;
        //         if (pos == body.size()) {
        //             bodyCtx.tempStore = "";
        //         }
        //         bodyCtx.tempStore = body.substr(pos);
        //     }
        // }
        // setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
        // conn->setState(Connection::SendResponse);
        // return;
        _parseTranfsferEncoding(conn);
        return;
    }
    _parseContentLength(conn);
}