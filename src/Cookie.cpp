#include "Cookie.h"

void Cookie::addSessionId(int socketfd) {
	CookieData newCookieData;
	newCookieData.socketfd = socketfd;
	newCookieData.name = toString(time(NULL)) + "_" + toString(socketfd); // unique name based on time and socketfd
	newCookieData.maxAge = 3600; // 1 hour
	newCookieData.expireTime = time(NULL) + newCookieData.maxAge; // current time + maxAge
	newCookieData.newSession = true;

	_sessionIds.push_back(newCookieData);
}

void Cookie::removeSessionId(int socketfd) {
	for (size_t i = 0; i < _sessionIds.size(); i++) {
		if (_sessionIds[i].socketfd == socketfd){
			_sessionIds.erase(_sessionIds.begin() + i);
			return;
		}
	}
}

void Cookie::checkSessionIds(int socketfd) {
	for (size_t i = 0; i < _sessionIds.size(); i++) {
		if (_sessionIds[i].socketfd == socketfd && _sessionIds[i].expireTime < time(NULL)) {
			// Session expired
			removeSessionId(socketfd);
			addSessionId(socketfd); // Create a new session
			return;
		}
	}
}

void Cookie::notNewSession(int socketfd) {
	for (size_t i = 0; i < _sessionIds.size(); i++) {
		if (_sessionIds[i].socketfd == socketfd) {
			_sessionIds[i].newSession = false; // Mark as not a new session
			return;
		}
	}
}
