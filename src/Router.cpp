#include "Router.h"
#include "HttpRequest.h"
#include <algorithm>
#include <iostream>
#include <map>

std::string Router::_matchLocations(HttpRequest req) {
    std::string route = "";
    std::vector< std::string > matches;
    std::set< std::string > _locs = _svrToLocs[req.headers["host"]];
    for (std::set< std::string >::iterator itLoc = _locs.begin(); itLoc != _locs.end(); itLoc++) {
        if (req.uri.compare(0, itLoc->length(), *itLoc) == 0)
            matches.push_back(*itLoc);
    }
    if (!matches.empty())
        route = req.headers["host"] + *std::max_element(matches.begin(), matches.end());
    return route;
}

Route Router::match(HttpRequest req) {
    // when host is not known, make it defaultSvr
    if (_svrs.find(req.headers["host"]) == _svrs.end())
        req.headers["host"] = _defaultSvr;

    std::string url = req.headers["host"] + req.uri;
    if (_urlToRoutes.find(url) != _urlToRoutes.end())
        return _urlToRoutes[url];

    url = _matchLocations(req);
    if (_urlToRoutes.find(url) != _urlToRoutes.end())
        return _urlToRoutes[url];

    return _urlToRoutes[req.headers["host"]];
}

void Router::add(std::string svrName, std::string prefix, std::string method, RouteConfig cfg) {
    if (_defaultSvr.empty())
        _defaultSvr = svrName;
    if (_svrs.find(svrName) == _svrs.end())
        _svrs.insert(svrName);

    if (_svrKnownPrefixPlusMethod[svrName].find(prefix + method) != _svrKnownPrefixPlusMethod[svrName].end())
        return;

    _urlToRoutes[svrName + prefix].cfg = cfg;
    _urlToRoutes[svrName + prefix].hdlrs[method] = _hdlrs[method];
    _svrToLocs[svrName].insert(prefix);
    _svrKnownPrefixPlusMethod[svrName].insert(prefix + method);
}

void Router::printUrls() {
    for (std::map< std::string, Route >::iterator it = _urlToRoutes.begin(); it != _urlToRoutes.end(); it++) {
        std::cout << it->first << std::endl;
    }
}

bool checkCGIRequest(HttpRequest& req, RouteConfig& cfg) {
    std::map< std::string, std::string > cgiMap = cfg.cgi;
    std::string path = req.uri;

    // Remove query string and fragment
    std::string::size_type queryPos = path.find('?');
    if (queryPos != std::string::npos)
        path = path.substr(0, queryPos);

    std::string::size_type fragPos = path.find('#');
    if (fragPos != std::string::npos)
        path = path.substr(0, fragPos);

    for (std::map< std::string, std::string >::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
        const std::string& ext = it->first;
        // std::string dotExt = "." + ext;
        std::string dotExt = ext;

        std::string::size_type pos = path.find(dotExt);
        if (pos == std::string::npos)
            continue;

        std::string::size_type next = pos + dotExt.size();
        if (next == path.size() || path[next] == '/' || path[next] == '?' || path[next] == '#') {
            return true;
        }
    }
    return false;
}
