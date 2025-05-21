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
        BodyContext& bodyCtx = conn->bodyCtx;
        if (!conn->_readBuf.size()) {
            conn->_bodyFinished = true;
            return;
        }
        std::string subContent;
        std::cout << "AT THE BEGINNING, TEMPSTORE IS: " << bodyCtx.tempStore << "READBUF IS: " << conn->_readBuf.data() << std::endl;
        std::string body = bodyCtx.tempStore + std::string(conn->_readBuf.data());
        // if (body.find("0\r\n\r\n") == std::string::npos) {
        //     setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
        //     conn->setState(Connection::SendResponse);
        //     return;
        // }
        size_t pos = 0;
        size_t holdPos = 0;
        while(pos < body.size()) {
            std::cout << "THE BODY IS: " << body << std::endl;
            if (bodyCtx.lengthOrBody == false) {
                holdPos = pos;
                pos = body.find("\r\n", holdPos);
                std::cout << "----------------TEST1----------------\n";
                if (pos == std::string::npos) {
                    std::cout << "----------------TEST2----------------\n";
                    bodyCtx.tempStore = body.substr(holdPos);
                    return;
                }
                std::istringstream iss(body.substr(holdPos, pos - holdPos));
                iss >> bodyCtx.contentLength;
                bodyCtx.lengthOrBody = true;
                pos += 2;
            }
            if (bodyCtx.lengthOrBody == true) {
                holdPos = pos;
                std::cout << "----------------TEST3----------------\n";
                if (holdPos == body.size()) {
                    std::cout << "----------------TEST4----------------\n";
                    bodyCtx.tempStore = "";
                    return;
                }
                pos = body.find("\r\n", holdPos);
                std::cout << "----------------TEST5----------------\n";
                if (pos == std::string::npos) {
                    bodyCtx.tempStore = body.substr(holdPos);
                    std::cout << "----------------TEST6----------------" << bodyCtx.tempStore << "----------------" << std::endl;;
                    return;
                }
                subContent = body.substr(holdPos, pos - holdPos);
                std::cout << "----------------TEST7----------------" << subContent << "----------------" << std::endl;;
                if (subContent.size() != bodyCtx.contentLength) {
                    std::cout << "----------------TEST8----------------\n";
                    setErrorResponse(conn->_response, 400, "Bad Request", conn->route.cfg);
                    conn->setState(Connection::SendResponse);
                    return;
                }
                conn->_tempBody += subContent;
                bodyCtx.lengthOrBody = false;
                pos += 2;
                std::cout << "----------------TEST9----------------\n";
                if (pos == body.size()) {
                    std::cout << "----------------TEST10----------------\n";
                    bodyCtx.tempStore = "";
                }
                bodyCtx.tempStore = body.substr(pos);
                std::cout << "TEMPSTORE IS: " << bodyCtx.tempStore << std::endl;
            }
            
        }
        conn->_bodyFinished = true;
        return;
    }
    _parseContentLength(conn);
}