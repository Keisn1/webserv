#include "test_ConnectionHandlerFixture.h"
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

// checking if the body is sent over the connection entirely
TEST_F(ConnHdlrTestWithBigResponseBody, firstTest) {
    int clientfd = _clientFdsAndConnFds[0].first;
    int connfd = _clientFdsAndConnFds[0].second;

    std::string request = "GET / HTTP/1.1\r\n"
                          "Host: test.com\r\n"
                          "\r\n";

    send(clientfd, request.c_str(), request.length(), 0);
    readUntilREADY_TO_WRITE(_ioNotifier, _connHdlr, connfd);

    std::string gotResponse;
    std::vector< t_notif > notifs = _ioNotifier->wait();
    while (notifs.size() > 0 && notifs[0].notif == READY_TO_WRITE) {
        _connHdlr->handleConnection(connfd, READY_TO_WRITE);
        while (true) {
            char buffer[1025];
            ssize_t r = recv(clientfd, &buffer[0], 1024, 0);
            if (r <= 0) {
                break;
            }
            buffer[r] = '\0';
            gotResponse += buffer;
        }
        notifs = _ioNotifier->wait();
    }

    std::string wantResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Length: " +
                               std::to_string(_body.length()) +
                               "\r\n"
                               "\r\n" +
                               _body;

    ASSERT_EQ(wantResponse, gotResponse);
    ASSERT_EQ(wantResponse.length(), gotResponse.length());
}
