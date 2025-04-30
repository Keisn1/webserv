#ifndef RESPONSEVALIDATOR_H
#define RESPONSEVALIDATOR_H

#include <iostream>
#include <string>
#include <set>

#include <HttpParser.h>
#include <Logger.h>

// Accept types: what types we can return
const std::set<std::string> supportedAcceptType = {
    "text/html",
    "application/json",
    "application/xml",
    "text/plain"
    // Add any other supported types here
};

// Accept-Encoding: what encodings we can return
const std::set<std::string> supportedEncodings = {
    "gzip",
    "deflate",
    "br",      // Brotli compression
    "identity" // No encoding
    // Add any other supported encodings here
};

// Accept-Language: what language we can return
const std::set<std::string> supportedLanguages = {
    "en",       // English
    "en-US",    // English (United States)
    "en-GB",    // English (United Kingdom)
    "fr",       // French
    "es",       // Spanish
    "zh-CN",    // Chinese (Simplified)
    "ja",       // Japanese
    "de",       // German
    "ru",       // Russian
    "ar"        // Arabic
    // Add more as needed
};

// Content types: what types of files we can accept
const std::set<std::string> supportedContentTypes = {
    "text/html",
    "text/plain",
    "application/json",
    "application/xml",
    "application/x-www-form-urlencoded",
    "multipart/form-data",
    "application/octet-stream"
    // Add more types as needed
};

typedef struct ValidatorInfo
{
    std::string statusCode;
	std::string statusMessage;
	bool isClosed = false;
	bool isRange = false;
	bool isChunked = false;
    bool hasError = false;
} ValidatorInfo;


class ResponseValidator
{
private:
    ValidatorInfo _validatorInfo;
public:
    void runValidator(HttpRequest& request, Logger& logger);
    ValidatorInfo getValidatorInfo(void);
};

#endif