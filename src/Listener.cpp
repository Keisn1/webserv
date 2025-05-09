#include "Listener.h"
#include "IConnectionHandler.h"
#include "IIONotifier.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/epoll.h>

Listener::Listener(ILogger& logger, IConnectionHandler* connHdlr, IIONotifier* epollMngr)
    : _logger(logger), _connHdlr(connHdlr), _ioNotifier(epollMngr) {}

Listener::~Listener() {
    delete _connHdlr;
    delete _ioNotifier;
}

void Listener::listen() {
    _isListening = true;
    while (_isListening) {
        int fd; // TODO: take not only one connection but #ready connections
        e_notif notif;
        int ready = _ioNotifier->wait(&fd, &notif);
        if (ready == 0)
            continue;
        if (!_isListening)
            break;

        _connHdlr->handleConnection(fd, notif);
    }
}

void Listener::stop() {
    _isListening = false;
    for (size_t i = 0; i < _socketfds.size(); i++)
        close(_socketfds[i]);
}

void Listener::add(int socketfd) {
    _ioNotifier->add(socketfd);
    _socketfds.push_back(socketfd);
}
