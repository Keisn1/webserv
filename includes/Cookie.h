#ifndef COOKIE_H
#define COOKIE_H

#include <ctime>
#include "TokenStream.h"

typedef struct CookieData
{
	std::string name;
	int socketfd; // socket file descriptor
	int maxAge; // in seconds
	int expireTime; // timestamp when the cookie expires
	bool newSession; // true if this cookie is for a new session
} CookieData;

class Cookie
{
private:
	std::vector<CookieData> _sessionIds;
public:
	void addSessionId(int socketfd);
	void removeSessionId(int socketfd);
	void checkSessionIds(int socketfd);
	void notNewSession(int socketfd);
};

#endif