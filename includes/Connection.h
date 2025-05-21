#ifndef CONNECTION_H
#define CONNECTION_H

#include "Buffer.h"
#include "HttpResponse.h"
#include "IHttpParser.h"
#include "ISender.h"
#include "ResponseWriter.h"
#include "Route.h"
#include "RouteConfig.h"
#include "SystemSender.h"
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

typedef struct BodyContext {
    size_t bytesReceived;
    size_t contentLength;
    std::string tempStore;
    bool lengthOrBody = false;
    BodyContext() : bytesReceived(0), contentLength(0), tempStore("") {}
} BodyContext;

typedef struct UploadContext {
    enum STATE { Validation, Initialising, Uploading, UploadFinished };
    size_t bytesUploaded;
    size_t contentLength;
    std::ofstream* file;
    bool fileExisted;
    UploadContext::STATE state;
    UploadContext()
        : bytesUploaded(0), contentLength(0), file(NULL), fileExisted(false), state(UploadContext::Validation) {}
} UploadContext;

struct CgiContext {
    pid_t cgiPid;               // Store CGI process ID
    int cgiPipeFd;              // Store CGI output pipe
    std::string cgiOutput;      // Accumulate CGI output
    RouteConfig cgiRouteConfig; // Store RouteConfig for response
    CgiContext() : cgiPid(-1), cgiPipeFd(-1) {}
};

class Connection {
  public:
    enum STATE { ReadingHeaders, Routing, Handling, HandleBadRequest, SendResponse, Reset, HandlingCgi };

  private:
    STATE _state;
    sockaddr_storage _addr;
    int _fd;
    std::string _addrPort;
    IHttpParser* _prsr;
    IResponseWriter* _wrtr;
    ISender* _sender;

  public:
    Route route;
    UploadContext uploadCtx;
    CgiContext cgiCtx;
    Buffer _readBuf;
    Buffer _sendBuf;
    BodyContext bodyCtx;
    bool _bodyFinished;
    std::string _tempBody;
    IHandler* _hdlr;

    ~Connection();

    Connection(sockaddr_storage addr, int fd, std::string addrPort, IHttpParser* prsr,
               ISender* sender = new SystemSender());
    STATE getState() const;
    void setState(Connection::STATE state);
    void readIntoBuf();
    void parseBuf();
    void sendResponse();
    void resetResponse();

    // getter and setter
    int getFileDes() const;
    HttpRequest getRequest();
    sockaddr_storage getAddr() const;
    std::string getReadBuf();
    std::string getAddrPort() { return _addrPort; }

    HttpRequest _request;
    HttpResponse _response;
};

#endif // CONNECTION_H
