#include <httpResposeWriter.h>

HttpResponse serializeResponse(const HttpResponse& response) {
    std::ostringstream oss;
    
    oss << response.httpVersion << " " << response.statusCode << " " << response.statusMessage << "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = response.headers.begin(); it != response.headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
    oss << "\r\n";
    std::string headers_part = oss.str();
    size_t total_size = headers_part.size() + response.body_size + 1;    
    char* result = new char[total_size];
    std::strcpy(result, headers_part.c_str());
    
    if (response.body && response.body_size > 0) {
        std::strncat(result, response.body, response.body_size);
    }
    result[total_size - 1] = '\0';
    
    return httpResponse{result, total_size};
}