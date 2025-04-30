#include "ResponseValidator.h"
#include <iostream>
#include <sstream>

void checkConnection(HttpRequest& request, Logger& logger, ValidatorInfo& _validatorInfo)
{	
	if (request.headers["Connection"] == "close") {
		_validatorInfo.isClosed = true;
	} else if (request.headers["Connection"] == "keep-alive") {
		_validatorInfo.isClosed = false;
	}
}

void checkAccept(HttpRequest& request, Logger& logger, ValidatorInfo& _validatorInfo)
{
    bool isValid = false;
    std::string value = request.headers["Accept"];
    std::istringstream iss(value);
    std::string token;
    std::vector<std::string> acceptTypes;

    // Split the Accept header by commas and trim spaces
    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));  // Trim leading spaces
        acceptTypes.push_back(token);
    }

    // Check if any of the accept types are valid by looking them up in the set
    for (size_t i = 0; i < acceptTypes.size(); i++) {
        std::string token2 = acceptTypes[i];
        size_t semicolon = token2.find(';');
        if (semicolon != std::string::npos) {
            token2 = token2.substr(0, semicolon);  // Remove anything after a semicolon
        }

        // If the type is in the supported set, mark it as valid
        if (supportedAcceptType.find(token2) != supportedAcceptType.end()) {
            isValid = true;
            break;  // Found a valid type, no need to check further
        }
    }

    // If no valid accept type is found, set the error and status code
    if (!isValid) {
        _validatorInfo.statusCode = "406";
        _validatorInfo.statusMessage = "Not Acceptable";
        _validatorInfo.hasError = true;
        logger.log("ERROR", "checkAccept: Not Acceptable");
    }
}

void checkAcceptEncoding(HttpRequest& request, Logger& logger, ValidatorInfo& _validatorInfo)
{
	bool isValid = false;
	std::string value = request.headers["Accept-Encoding"];
	std::istringstream iss(value);
	std::string token;
	std::vector<std::string> acceptEncodings;
	while (std::getline(iss, token, ',')) {
		token.erase(0, token.find_first_not_of(" "));
		acceptEncodings.push_back(token);
	}
	for (size_t i = 0; i < acceptEncodings.size(); i++) {
		std::string token2 = acceptEncodings[i];
		size_t semicolon = token2.find(';');
		if (semicolon != std::string::npos) {
			token2 = token2.substr(0, semicolon);
		}
		// If the type is in the supported set, mark it as valid
        if (supportedEncodings.find(token2) != supportedEncodings.end()) {
            isValid = true;
            break;  // Found a valid type, no need to check further
        }
	}
	if (isValid == false) {
		_validatorInfo.statusCode = "406";
		_validatorInfo.statusMessage = "Not Acceptable";
		_validatorInfo.hasError = true;
		logger.log("ERROR", "checkAcceptEncoding: Not Acceptable");
	}
}

void	checkAcceptLanguage(HttpRequest& request, Logger& logger, ValidatorInfo& _validatorInfo)
// IF WE CAN NOT FINT THE LANGUAGE, WE TREATED AS NOT ACCEPTABLE, BUT WE CAN CHANGE IT AFTERWARDS
// ACTUALLY, THE REAL CASE IS SET THE LANGUAGE INTO DEFAULT LANGUAGE IF WE CAN NOT FIND THE LANGUAGE
{
	std::string result;
	bool isValid = false;
	std::string value = request.headers["Accept-Language"];
	std::istringstream iss(value);
	std::string token;
	std::vector<std::string> acceptLanguages;
	while (std::getline(iss, token, ',')) {
		token.erase(0, token.find_first_not_of(" "));
		acceptLanguages.push_back(token);
	}
	for (size_t i = 0; i < acceptLanguages.size(); i++) {
		std::string token2 = acceptLanguages[i];
		size_t semicolon = token2.find(';');
		if (semicolon != std::string::npos) {
			token2 = token2.substr(0, semicolon);
		}
		// If the type is in the supported set, mark it as valid
		if (supportedLanguages.find(token2) != supportedLanguages.end()) {
			isValid = true;
			break;  // Found a valid type, no need to check further
		}
	}
	if (isValid == false) {
		_validatorInfo.statusCode = "406";
		_validatorInfo.statusMessage = "Not Acceptable";
		_validatorInfo.hasError = true;
		logger.log("ERROR", "checkAcceptLanguage: Not Acceptable");
	}
}

void checkRange(HttpRequest& request, Logger& logger, ValidatorInfo& _validatorInfo)
{
	std::string token = request.headers["Range"].substr(request.headers["Range"].find('=') + 1);
	std::istringstream iss(token);
	std::string range;
	std::vector<std::string> ranges;
	while (std::getline(iss, range, ',')) {
		range.erase(0, range.find_first_not_of(" "));
		ranges.push_back(range);
	}
	for (size_t i = 0; i < ranges.size(); i++) {
		int start = atoi(ranges[i].substr(0, ranges[i].find('-')).c_str());
		int end = atoi(ranges[i].substr(ranges[i].find('-') + 1).c_str());
		if (start > end) {
			_validatorInfo.statusCode = "416";
			_validatorInfo.statusMessage = "Requested Range Not Satisfiable";
			_validatorInfo.hasError = true;
			logger.log("ERROR", "checkRange: Requested Range Not Satisfiable");
		}
		if (start < 0 || end < 0) {
			_validatorInfo.statusCode = "416";
			_validatorInfo.statusMessage = "Requested Range Not Satisfiable";
			_validatorInfo.hasError = true;
			logger.log("ERROR", "checkRange: Requested Range Not Satisfiable");
		}
		_validatorInfo.isRange = true;
	}
}

std::string checkContentType(HttpRequest& request, Logger& logger, ValidatorInfo& _validatorInfo)
{
	std::string result;
	std::string contentType = request.headers["Content-Type"];
	if (supportedContentTypes.find(contentType) != supportedContentTypes.end()) {
		result = contentType;
	} else {
		_validatorInfo.statusCode = "415";
		_validatorInfo.statusMessage = "Unsupported Media Type";
		_validatorInfo.hasError = true;
		logger.log("ERROR", "checkContentType: Unsupported Media Type");
	}
}

void checkTransferEncoding(Logger& logger, ValidatorInfo& _validatorInfo)
{
	logger.log("INFO", "checkTransferEncoding: Transfer-Encoding: chunked");
	_validatorInfo.isChunked = true;
}

void ResponseValidator::runValidator(HttpRequest& request, Logger& logger)
{	
	for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); ++it) {
		if (it->first == "connection") {
			checkConnection(request, logger, _validatorInfo);
		}
		if (it->first == "upgrade") {
			;//WE DO NOT SUPPORT UPGRADE SO FAR BUT WE CAN ADD IT IF WE NEED TO
		}
		if (it->first == "accept") {
			checkAccept(request, logger, _validatorInfo);
		}
		if (it->first == "accept-encoding") {
			checkAcceptEncoding(request, logger, _validatorInfo);
		}
		if (it->first == "accept-language") {
			checkAcceptLanguage(request, logger, _validatorInfo);
		}
		if (it->first == "range") {
				checkRange(request, logger, _validatorInfo);
		}
		if (it->first == "content-type") {
			checkContentType(request, logger, _validatorInfo);
		}
		if (it->first == "transfer-encoding") {
			checkTransferEncoding(logger, _validatorInfo);
		}
		if (it->first == "expect") {
			;//WE DO NOT SUPPORT EXPECT SO FAR BUT WE CAN ADD IT IF WE NEED TO
		}
		if (it->first == "cookie") {
			;//WE DO NOT SUPPORT COOKIE SO FAR BUT WE CAN ADD IT IF WE NEED TO
		}
		// THE NEXT STEP IS TO GENERATE THE RESPONSE MESSAGE
	}
}

ValidatorInfo ResponseValidator::getValidatorInfo(void)
{
	return _validatorInfo;
}