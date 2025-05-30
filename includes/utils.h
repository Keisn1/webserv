#ifndef UTILS_H
#define UTILS_H

#include "ConfigStructure.h"
#include "IIONotifier.h"
#include <set>
#include <sstream>
#include <string>

void getAddrInfoHelper(const char* node, const char* port, int protocol, struct addrinfo** addrInfo);
int newSocket(struct addrinfo* addrInfo);
int newSocket(std::string ip, std::string port, int protocol);
int newListeningSocket(struct addrinfo* addrInfo, int backlog);
int newListeningSocket(std::string ip, std::string port, int protocol, int backlog);
int getPort(int socketfd);
std::string getIpv4String(struct sockaddr_in* addr_in);
std::string getIpv6String(struct sockaddr_in6* addr);
std::string getAddressAndPort(int socketfd);
void mustGetAddrInfo(std::string addr, std::string port, struct addrinfo** svrAddrInfo);
void mustTranslateToRealIps(std::vector< ServerConfig >& svrCfgs);
std::set< std::pair< std::string, std::string > > fillAddrAndPorts(std::vector< ServerConfig > svrCfgs);

template < typename T >
std::string to_string(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string toLower(const std::string& str);

void printTimeval(const timeval& tv);
void printNotif(const t_notif& notif);

#endif // UTILS_H
