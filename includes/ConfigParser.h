#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <fstream>
#include <fcntl.h>

#include <TokenStream.h>
#include <IConfigParser.h>

class ConfigParser : public IConfigParser {
  private:
    Context _ast;
    ServerConfig _serverConfig;
    void _makeAst(const std::string& filename);
    void _makeServerConfig();
    void _parseServerContext(const Context& serverContext);
    LocationConfig _parseLocationContext(const Context& locationContext);
    void _processDirectives(const Context& context, ServerConfig& config);
    void _validateServerContext(const Context& context);
    
  public:
    ConfigParser();
    ~ConfigParser();
    ServerConfig getServerConfig(const std::string& filename);
    Context getAst(const std::string& filename);
};

#endif // CONFIGPARSER_H