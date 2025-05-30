#include "test_stubs.h"
#include <Router.h>

Router newRouterTest() {
    std::string defaultSvr;
    std::set< std::string > svrs;
    std::map< std::string, std::set< std::string > > svrToLocs;
    std::map< std::string, std::set< std::string > > routeToAllowedMethods;

    defaultSvr = "example.com";

    svrs = {"example.com", "www.example.com", "test.com",  "www.test.com",
            "test2.com",   "test3.com",       "test5.com", "example2.com"};

    svrToLocs = {
        {"www.example.com", {"/css/"}},
        {"example2.com", {"/"}},
        {"example.com", {"/images/", "/css/scripts/", "/css/", "/css/styles/"}},
        {"test.com", {"/css/", "/js/", "/images/"}},
        {"www.test.com", {"/css/", "/js/", "/images/"}},
        {"test2.com", {}},
        {"test3.com", {"/"}},
        {"test5.com", {"/google/"}},

    };

    std::map< std::string, IHandler* > hdlrs = {
        {"GET", new StubHandler("GET")},
        {"POST", new StubHandler("POST")},
        {"DELETE", new StubHandler("DELETE")},
        {"CGI", new StubHandler("CGI")},
    };

    std::map< std::string, Route > urlToRoutes;
    urlToRoutes = {
        {"example.com/post_body", {{{"POST", hdlrs["POST"]}}, {"/var/www/html", {}, {}, oneMB, false, {}, {}}}},
        {"example2.com/",
         {{{"GET", hdlrs["GET"]}},
          {"/example2/www/html", {"index.html", "index.htm"}, {{404, "/custom_404.html"}}, oneMB, false, {}, {}}}},
        {"www.example.com/css/",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}}, {"/dataSecond/static", {}, {}, oneMB, false, {}, {}}}},

        {"www.example.com", {{{"GET", hdlrs["GET"]}}, {"/var/worldwideweb/html", {}, {}, oneMB, false, {}, {}}}},

        {"www.test.com/css/",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
          {"/data/static", {"index.html", "index.htm"}, {}, oneMB, false, {}, {}}}},

        {"example.com/css/scripts/",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
          {"/data/scripts", {}, {}, 12 * oneMB, false, {}, {}}}},
        {"example.com/css/styles/", {{{"GET", hdlrs["GET"]}}, {"/data/extra", {}, {}, oneKB, false, {}, {}}}},
        {"test3.com/", {{{"DELETE", hdlrs["DELETE"]}}, {"/test3/www/html", {}, {}, oneMB, false, {}, {}}}},
        {"test.com/images/",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
          {"/data2",
           {"index.html", "index.htm"},
           {{404, "/custom_404.html"},
            {500, "/custom_50x.html"},
            {502, "/custom_50x.html"},
            {503, "/custom_50x.html"},
            {504, "/custom_50x.html"}},
           oneMB,
           false,
           {},
           {}}}},
        {"test.com/js/",
         {{{"GET", hdlrs["GET"]}}, {"/data/scripts", {"index.html", "index.htm"}, {}, oneMB, false, {}, {}}}},
        {"example.com/images/",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}, {"CGI", hdlrs["CGI"]}},
          {"/data", {}, {}, oneMB, false, {}, {}}}},
        {"example.com/css/",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}}, {"/data/static", {}, {}, oneMB, false, {}, {}}}},
        {"example.com", {{{"GET", hdlrs["GET"]}}, {"/var/www/html", {}, {}, oneMB, false, {}, {}}}},
        {"test.com",
         {{{"GET", hdlrs["GET"]}, {"POST", hdlrs["POST"]}, {"DELETE", hdlrs["DELETE"]}},
          {"/var/www/secure", {"index.html", "index.htm"}, {}, oneMB, false, {}, {}}}},
        {"test2.com", {{{"GET", hdlrs["GET"]}}, {"/usr/share/nginx/html", {}, {}, oneMB, false, {}, {}}}},
        {"test5.com",
         {{{"GET", hdlrs["GET"]}}, {"/test5/www/html", {}, {}, oneMB, false, {}, {301, "https://www.google.com"}}}}};

    return Router(hdlrs, defaultSvr, svrs, svrToLocs, urlToRoutes);
}
