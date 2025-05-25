#ifndef COOKIE_H
#define COOKIE_H

#include <ctime>
#include "TokenStream.h"
#include "Connection.h"

typedef struct CookieData
{
	std::string name;
	int maxAge; // in seconds
	int expireTime; // timestamp when the cookie expires
	bool newSession; // true if this cookie is for a new session
} CookieData;

class Cookie
{
private:
	std::vector<CookieData> _sessionIds;
public:
	std::string addSessionId(void);
	void removeSessionId(std::string sessionId);
	std::string checkSessionIds(Connection* conn);
	void notNewSession(std::string sessionId);
};

#endif