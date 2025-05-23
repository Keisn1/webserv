#include "ILogger.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

bool checkValidMethod(const std::string& method) {
    return method == "GET" || method == "POST" || method == "PUT" || method == "DELETE" || method == "HEAD" ||
           method == "OPTIONS" || method == "PATCH";
}

bool checkValidVersion(const std::string& version) {
    double ver = 0;
    if (version.empty()) {
        return false;
    }
    if (version.find("HTTP/") != 0) {
        return false;
    }
    std::string verStr = version.substr(5);
    if (verStr.size() != 3) {
        return false;
    }
    if (!isdigit(verStr[0]) || verStr[1] != '.' || !isdigit(verStr[2])) { // TODO: why check 2 times for a "."?
        return false;
    }
    std::istringstream iss(verStr);
    iss >> ver;
    if (ver < 1 || ver > 2) { //I CHANGED THIS FROM 0 TO 1, BECAUSE HTTP/0.9 IS NOT SUPPORTED
        return false;
    }
    return true;
}

bool isValidTokenChar(char c) {
    return isalnum(c) || c == '-' || c == '_' || c == '.';
}

bool checkOptionalCharset(const std::string& str) {
    size_t pos = str.find("charset=");
    if (pos == std::string::npos)
        return true;

    std::string charset = str.substr(pos + 8);
    charset.erase(0, charset.find_first_not_of(" "));
    if (charset.empty())
        return false;

    for (size_t i = 0; i < charset.size(); i++) {
        if (!isValidTokenChar(charset[i]))
            return false;
    }
    return true;
}

bool checkOptionalBoundary(const std::string& str) {
    size_t pos = str.find("boundary=");
    if (pos == std::string::npos)
        return true;

    std::string boundary = str.substr(pos + 9);
    boundary.erase(0, boundary.find_first_not_of(" "));
    if (boundary.empty())
        return false;

    for (size_t i = 0; i < boundary.size(); i++) {
        if (!isValidTokenChar(boundary[i]))
            return false;
    }
    return true;
}

bool isValidContentType(const std::string& str) {
    size_t slash = str.find('/');
    if (slash == std::string::npos || slash == 0 || slash == str.size() - 1)
        return false;

    std::string type = str.substr(0, slash);
    if (type != "application" && type != "text" && type != "image" && type != "audio" &&
        type != "video" && type != "multipart" && type != "font" && type != "model" &&
        type != "message" && type != "example")
        return false;

    // Only validate charset/boundary if present
    if (!checkOptionalCharset(str))
        return false;
    if (type == "multipart" && (!checkOptionalBoundary(str) || 
        str.find("boundary=") == std::string::npos ||
        str.find(';') == std::string::npos))
        return false;
    return true;
}

bool checkQValue(const std::string& str) {
    size_t pos = 0;

    while (pos < str.size()) {
        // Look for 'q'
        if (str[pos] == 'q') {
            ++pos;

            // Skip whitespace
            while (pos < str.size() && str[pos] == ' ')
                ++pos;

            // Check for '='
            if (pos >= str.size() || str[pos] != '=')
                return false;
            ++pos;

            // Skip whitespace again
            while (pos < str.size() && str[pos] == ' ')
                ++pos;

            // Must be a digit starting the qvalue
            if (pos >= str.size() || !isdigit(str[pos]))
                return false;

            // Parse q-value as a double
            std::string qvalue;
            while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.'))
                qvalue += str[pos++];

            std::istringstream iss(qvalue);
            double q = 0;
            iss >> q;

            if (iss.fail() || !iss.eof())
                return false;

            // Range check
            if (q < 0.0 || q > 1.0)
                return false;
        } else {
            ++pos;
        }
    }
    return true;
}

bool isValidHost(std::string str) {
    size_t colon = str.find(':');
    if (colon != std::string::npos) {
        std::string host = str.substr(0, colon);
        std::string port = str.substr(colon + 1);
        if (host.empty() || port.empty()) {
            return false;
        }
        for (size_t i = 0; i < port.size(); i++) {
            if (!isdigit(port[i])) {
                return false;
            }
        }
    } else {
        if (str.empty()) {
            return false;
        }
    }
    return true;
}

bool isValidAccept(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t slash = token2.find('/');
        if (slash == std::string::npos || slash == 0 ||
            slash == token2.size() - 1) // the char '/' must be in the middle of the string
            return false;
        std::string type = token2.substr(0, slash);
        if (type != "application" && type != "text" && type != "image" && type != "audio" && type != "video" &&
            type != "multipart" && type != "font" && type != "model" && type != "message" && type != "example") {
            return false;
        }
        // Only validate charset/boundary if present
        if (!checkOptionalCharset(token2)) {
            return false;
        }
        if (!checkOptionalBoundary(token2)) {
            return false;
        }
        if (!checkQValue(token2)) {
            return false;
        }
    }
    return true;
}

bool isValidAcceptEncoding(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    std::string token3;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t semicolon = token2.find(';');
        if (semicolon != std::string::npos) {
            token3 = token2.substr(0, semicolon);
        }
        else {
            token3 = token2;
        }
        if (token3 != "gzip" && token3 != "deflate" && token3 != "br" && token3 != "compress" && token3 !=
        "identity") {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidAcceptLanguage(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    std::string lang;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t breakpoint;
        if (token2.find(';') != std::string::npos && token2.find('-') != std::string::npos) {
            if (token2.find(';') < token2.find('-')) {
                breakpoint = token2.find(';');
            } else {
                breakpoint = token2.find('-');
            }
        } else if (token2.find(';') != std::string::npos) {
            breakpoint = token2.find(';');
        } else if (token2.find('-') != std::string::npos) {
            breakpoint = token2.find('-');
        } else {
            breakpoint = token2.length();
        }
        lang = token2.substr(0, breakpoint);
        if (lang.empty() || lang.length() != 2) {
            return false;
        }
        if (token2.find('-') != std::string::npos) {
            std::string country = token2.substr(token2.find('-') + 1, token2.find(';') - token2.find('-') - 1);
            if (country.empty()) {
                return false; // A region code must follow the '-'
            }
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidCookie(std::string str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ';')) {
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t equal = token2.find('=');
        if (equal == std::string::npos || equal == 0 ||
            equal == token2.size() - 1) // the char '=' must be in the middle of the string
            return false;
        std::string key = token2.substr(0, equal);
        if (key.empty()) {
            return false;
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidRange(const std::string& str) {
    size_t pos = str.find("bytes=");
    if (pos == std::string::npos) {
        return false;
    }
    std::string range = str.substr(pos + 6);
    std::vector<std::string> tokens;
    std::istringstream iss(range);
    std::string token;
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        tokens.push_back(token);
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string token2 = tokens[i];
        size_t dash = token2.find('-');
        if (dash == std::string::npos || dash == 0 ||
            dash == token2.size() - 1) // the char '-' must be in the middle of the string
            return false;
        std::string start = token2.substr(0, dash);
        size_t semicolon = token2.find(';');
        std::string end = token2.substr(dash + 1, semicolon - dash - 1);
        if (start.empty() || end.empty()) {
            return false;
        }
        for (size_t j = 0; j < start.size(); j++) {
            if (!isdigit(start[j])) {
                return false;
            }
        }
        for (size_t k = 0; k < end.size(); k++) {
            if (!isdigit(end[k])) {
                return false;
            }
        }
        if (checkQValue(token2) == false) {
            return false;
        }
    }
    return true;
}

bool isValidContentLength(const std::string& str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool specificHeaderValidation(
    // so far, I added these functions, but if we need more, I can add them
    const std::string& key, const std::string& value, ILogger& logger) {

    if (key == "host") {
        if (!isValidHost(value)) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Host header");
            return false;
        }
    }
    if (key == "content-length") {
        if (!isValidContentLength(value)) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Content-Length header");
            return false;
        }
    }
    if (key == "transfer-encoding") {
        const std::string chunked = "chunked";
        if (value.length() < chunked.length() || value.substr(value.length() - chunked.length()) != chunked) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Transfer-Encoding header");
            return false;
        }
    }
    if (key == "connection") {
        if (value != "keep-alive" && value != "close") {
            logger.log("ERROR", "specificHeaderValidation: Invalid Connection header");
            return false;
        }
    }
    if (key == "content-type") {
        if (isValidContentType(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Content-Type header");
            return false;
        }
    }
    if (key == "accept") {
        if (isValidAccept(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Accept header");
            return false;
        }
    }
    if (key == "accept-encoding") {
        if (isValidAcceptEncoding(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Accept-Encoding header");
            return false;
        }
    }
    if (key == "accept-language") {
        if (isValidAcceptLanguage(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Accept-Language header");
            return false;
        }
    }
    if (key == "cookie") {
        if (isValidCookie(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Cookie header");
            return false;
        }
    }
    if (key == "range") {
        if (isValidRange(value) == false) {
            logger.log("ERROR", "specificHeaderValidation: Invalid Range header");
            return false;
        }
    }
    return true;
}
