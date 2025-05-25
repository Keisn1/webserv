#include "Cookie.h"
#include <cstdlib>

std::string Cookie::addSessionId(void) {
	srand(static_cast<unsigned int>(time(NULL)));
	CookieData newCookieData;
	for (size_t i = 0; i < 10; i++)
		newCookieData.name += static_cast<char>('a' + rand() % 26); // Generate a random lowercase letter
	newCookieData.maxAge = 3600; // 1 hour
	newCookieData.expireTime = time(NULL) + newCookieData.maxAge; // current time + maxAge
	newCookieData.newSession = true;

	_sessionIds.push_back(newCookieData);
	return newCookieData.name;
}

void Cookie::removeSessionId(std::string sessionId) {
	for (size_t i = 0; i < _sessionIds.size(); i++) {
		if (_sessionIds[i].name == sessionId) {
			_sessionIds.erase(_sessionIds.begin() + i);
			return;
		}
	}
}

std::string Cookie::checkSessionIds(Connection* conn) {
	std::string sessionId;
	if (conn->_request.headers.find("Cookie") == conn->_request.headers.end()) {
		// No cookie header found, create a new session
		sessionId = addSessionId();
		return sessionId;
	}

	if (conn->_request.headers["Cookie"].find("sessionId") == std::string::npos) {
		// No sessionId found in cookie header, create a new session
		sessionId = addSessionId();
		return sessionId;
	}

	bool sessionFound = false;
	std::string cookieHeader = conn->_request.headers["Cookie"];
	std::string sessionId;

	size_t start = cookieHeader.find("sessionId=");
	if (start != std::string::npos) {
		start += 10; // Length of "sessionId="
		size_t end = cookieHeader.find(";", start); // Look for the next semicolon after sessionId=
		if (end == std::string::npos) {
			sessionId = cookieHeader.substr(start); // sessionId is last in the cookie string
		} else {
			sessionId = cookieHeader.substr(start, end - start);
		}
	}
	size_t i;
	for (i = 0; i < _sessionIds.size(); i++) {
		if (_sessionIds[i].name == sessionId) {
			sessionFound = true;
			break;
		}
	}

	if (!sessionFound) {
		// sessionId not found in existing sessionIds, create a new session
		sessionId = addSessionId();
		return sessionId;
	} else {
		if (_sessionIds[i].expireTime < time(NULL)) {
			// Session has expired, create a new session
			sessionId = addSessionId();
			return sessionId;
		} else {
			_sessionIds[i].expireTime = time(NULL) + _sessionIds[i].maxAge; // Reset expiration time
			return _sessionIds[i].name;
		}
	}
}

void Cookie::notNewSession(std::string sessionId) {
	for (size_t i = 0; i < _sessionIds.size(); i++) {
		if (_sessionIds[i].name == sessionId) {
			_sessionIds[i].newSession = false; // Mark as not a new session
			return;
		}
	}
}
