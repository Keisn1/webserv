#ifndef CONNECTION_H
#define CONNECTION_H

#include "HttpResponse.h"
#include "IHttpParser.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Connection {
  public:
    enum STATE { ReadingHeaders, Handling, HandleBadRequest, SendResponse };

  private:
    STATE _state;
    sockaddr_storage _addr;
    std::string _buf;
    int _fd;
    IHttpParser* _prsr;

  public:
    ~Connection();
    Connection(sockaddr_storage addr, int fd, IHttpParser* prsr);
    STATE getState() const;
    void readIntoBuf();
    void parseBuf();
    int getFileDes() const;
    sockaddr_storage getAddr() const;
    HttpResponse _response;
    void setState(Connection::STATE state) { _state = state; }

    // THIS IS TEMP ADD
    size_t preBodySize;
    std::string body;
};

#endif // CONNECTION_H
