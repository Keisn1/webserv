#include "ConnectionHandler.h"
#include "BadRequestHandler.h"
#include "BodyParser.h"
#include "HttpParser.h"
#include "IIONotifier.h"
#include "handlerUtils.h"
#include "httpParsing.h"
#include "logging.h"
#include "utils.h"
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

std::map< std::string, std::string > parseCookies(const std::string& cookieHeader) {
    std::map< std::string, std::string > cookies;
    std::stringstream ss(cookieHeader);
    std::string item;

    while (std::getline(ss, item, ';')) {
        std::string::size_type pos = item.find('=');
        if (pos != std::string::npos) {
            std::string key = item.substr(0, pos);
            std::string value = item.substr(pos + 1);

            // Trim leading spaces from the key
            key.erase(0, key.find_first_not_of(" \t"));
            // Optionally, trim spaces from the value too
            value.erase(0, value.find_first_not_of(" \t"));

            cookies[key] = value;
        }
    }

    return cookies;
}

ConnectionHandler::ConnectionHandler(std::map< std::string, IRouter* > routers, ILogger& l, IIONotifier& io)
    : _routers(routers), _logger(l), _ioNotifier(io) {
    _bodyPrsr = new BodyParser();
}

ConnectionHandler::~ConnectionHandler(void) {
    delete _bodyPrsr;
    for (std::map< int, Connection* >::iterator it = _connections.begin(); it != _connections.end(); it++)
        delete it->second;

    for (std::map< std::string, IRouter* >::iterator it = _routers.begin(); it != _routers.end(); it++)
        delete it->second;
}

void ConnectionHandler::_updateNotifier(Connection* conn) {
    int connfd = conn->getFileDes();
    switch (conn->getState()) {
    case Connection::ReadingHeaders:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::Redirecting:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::Handling:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::HandleBadRequest:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    case Connection::SendResponse:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    case Connection::Reset:
        _ioNotifier.modify(connfd, READY_TO_READ);
        break;
    case Connection::HandlingCgi:
        _ioNotifier.modify(connfd, READY_TO_WRITE);
        break;
    default:
        break;
    }
}

void ConnectionHandler::_addClientConnection(int connfd, struct sockaddr_storage theirAddr, std::string addrPort) {
    logConnection(_logger, theirAddr);
    Connection* conn = new Connection(theirAddr, connfd, addrPort, new HttpParser(_logger));
    _connections[connfd] = conn;
    _ioNotifier.add(connfd);
}

void ConnectionHandler::_removeConnection(int connfd) {
    logDisconnect(_logger, _connections[connfd]->getAddr());
    _ioNotifier.del(connfd);
    delete _connections[connfd];
    _connections.erase(connfd);
}

int ConnectionHandler::_acceptNewConnection(int socketfd) {
    struct sockaddr_storage theirAddr;
    int addrlen = sizeof(theirAddr);
    int fd = accept(socketfd, (struct sockaddr*)&theirAddr, (socklen_t*)&addrlen);
    if (fd < 0) {
        _logger.log("ERROR", "accept: " + std::string(strerror(errno)));
        return -1;
    }
    _addClientConnection(fd, theirAddr, getAddressAndPort(socketfd));
    return fd;
}

void ConnectionHandler::_handleState(Connection* conn) {
    HttpResponse resp;
    IHandler* hdlr;
    IRouter* router;
    Route route;
    bool continueProcessing = true;
    std::string sessionID;
    while (continueProcessing) {
        Connection::STATE currentState = conn->getState();
        switch (currentState) {
        case Connection::ReadingHeaders:
            _logger.log("DEBUG", std::string(conn->_readBuf.data(), conn->_readBuf.size()));
            conn->parseBuf();
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::Routing:
            if (!checkValidVersion(conn->_request.version)) {
                setErrorResponse(conn->_response, 505, RouteConfig());
                conn->setState(Connection::SendResponse);
                break;
            }
            conn->_request.uri = parseOutProtocolAndHost(conn->_request.uri);
            if (conn->_request.uri.empty()) {
                setErrorResponse(conn->_response, 400, RouteConfig());
                conn->setState(Connection::SendResponse);
                break;
            }
            conn->_request.query = extractQuery(conn->_request.uri);
            conn->_request.uri = normalizePath("", conn->_request.uri);

            if (conn->_request.uri == "..") {
                setErrorResponse(conn->_response, 403, RouteConfig());
                conn->setState(Connection::SendResponse);
                break;
            }

            if (conn->_request.uri.empty())
                conn->_request.uri = "/";

            if (conn->_request.headers.find("cookie") != conn->_request.headers.end()) {
                conn->cookies = parseCookies(conn->_request.headers["cookie"]);
            }

            ///////////////
            // cookies   //
            ///////////////

            if (conn->cookies.find("sessionid") != conn->cookies.end()) {
                // 1. retrieve sessionID
                sessionID = conn->cookies.at("sessionid");
                if (_sessionIdsDataBase.find(sessionID) == _sessionIdsDataBase.end()) {
                    // 1. Create sessionId
                    sessionID = getSessionID();
                    // 2. Save sessionId in some data structure that is persistent in the server (as long as the server
                    // runs)
                    _sessionIdsDataBase[sessionID] = 1;
                    // 3. Log the session information (countVisits, method, uri, timestamp, maybe max-age)
                    _logger.log("INFO", "session created: " + sessionID);
                    // 4. Set the cookie (response / ResponseWriter)
                    setHeader(conn->_response, "Set-Cookie", "sessionid=" + sessionID);
                } else {
                    _sessionIdsDataBase[sessionID] += 1;
                }
            } else {
                // 1. Create sessionId
                sessionID = getSessionID();
                // 2. Save sessionId in some data structure that is persistent in the server (as long as the server
                // runs)
                _sessionIdsDataBase[sessionID] = 1;
                // 3. Log the session information (countVisits, method, uri, timestamp, maybe max-age)
                _logger.log("INFO", "session created: " + sessionID);
                // 4. Set the cookie (response / ResponseWriter)
                setHeader(conn->_response, "Set-Cookie", "sessionid=" + sessionID);
            }
            // 3. Log the session information (countVisits, method, uri, timestamp, maybe max-age)
            _logger.log("INFO", "Number of requests from sessionID " + sessionID + " = " +
                                    to_string(_sessionIdsDataBase[sessionID]));

            ///////////////
            // cookies   //
            ///////////////

            router = _routers[conn->getAddrPort()];
            route = router->match(conn->getRequest());
            conn->route = route;
            conn->checkRoute();
            break;
        case Connection::Redirecting:
            _bodyPrsr->parse(conn);
            if (conn->getState() == Connection::SendResponse)
                break;

            if (conn->_bodyFinished) {
                int code = conn->route.cfg.redirect.first;
                setHeader(conn->_response, "Location", conn->route.cfg.redirect.second);
                setResponse(conn->_response, code, "", 0, NULL);
                conn->setState(Connection::SendResponse);
                break;
            }
            continueProcessing = false;
            break;
        case Connection::Handling:
            _bodyPrsr->parse(conn);
            if (conn->getState() == Connection::SendResponse)
                break;
            conn->_hdlr->handle(conn, conn->_request, conn->route.cfg);
            continueProcessing = (conn->getState() != currentState);
            break;
        case Connection::HandleBadRequest:
            hdlr = new BadRequestHandler();
            hdlr->handle(conn, HttpRequest(), RouteConfig());
            delete hdlr;
            continueProcessing = (conn->getState() != currentState);
            break;
        default:
            continueProcessing = false;
            break;
        }
    }
    _updateNotifier(conn);
}

void ConnectionHandler::_onSocketRead(Connection* conn) {
    if (conn->readIntoBuf() <= 0) // EOF or error
        return _removeConnection(conn->getFileDes());
    _handleState(conn);
    return;
}

void ConnectionHandler::_onSocketWrite(Connection* conn) {
    if (conn->getState() == Connection::HandlingCgi) {
        conn->_hdlr->handle(conn, conn->_request, conn->route.cfg);
    }

    if (conn->getState() == Connection::SendResponse) {
        ssize_t r = conn->sendResponse();
        if (r < 0) {
            _removeConnection(conn->getFileDes());
            return;
        }

        if (conn->_response.statusCode >= 400) {
            _removeConnection(conn->getFileDes());
            return;
        }
        if (conn->getState() == Connection::Reset) {
            conn->resetResponse();
            conn->resetCGI();
            conn->_bodyFinished = false;
            conn->setState(Connection::ReadingHeaders);
            _handleState(conn); // possibly data inside Connection
            return;
        }
    }
}

int ConnectionHandler::handleConnection(int fd, e_notif notif) {
    _logger.log("DEBUG", "Handling fd: " + to_string(fd));
    if (_connections.find(fd) == _connections.end()) {
        return _acceptNewConnection(fd);
    }

    Connection* conn = _connections[fd];
    switch (notif) {
    case READY_TO_READ:
        _logger.log("DEBUG", "Ready to read");
        _onSocketRead(conn);
        break;
    case READY_TO_WRITE:
        _logger.log("DEBUG", "Ready to write");
        _onSocketWrite(conn);
        break;
    case CLIENT_HUNG_UP:
        _logger.log("DEBUG", "Client hung up");
        _removeConnection(fd);
        break;
    case BROKEN_CONNECTION:
        _logger.log("DEBUG", "Broken Connection");
        _removeConnection(fd);
        break;
    case TIMEOUT:
        _logger.log("INFO", "Timeout");
        setErrorResponse(conn->_response, 408, conn->route.cfg);
        conn->setState(Connection::SendResponse);
        _updateNotifier(conn);
        break;
    }
    return fd;
}
