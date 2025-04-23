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
    LocationConfig _parseLocationContext(const Context& locationContext);
    void _processServerDirectives(const Context& context, ServerConfig& config);

    void _parseListen(const Directive& directive, ServerConfig& config);
    void _parseServerNames(const Directive& directive, ServerConfig& config);
    void _parseRoot(const Directive& directive, CommonConfig& config);
    void _parseClientMaxBodySize(const Directive& directive, CommonConfig& config);
    void _parseAllowMethods(const Directive& directive, CommonConfig& config);
    void _parseIndex(const Directive& directive, CommonConfig& config);
    
    void _parseServerContext(const Context& serverContext);
    void _validateServerContext(const Context& context);
    void _makeAst(const std::string& filename);
    void _makeServerConfig();
    
  public:
    ConfigParser(const std::string& filename);
    ~ConfigParser();
    ServerConfig getServerConfig();
    Context getAst();
};

#endif // CONFIGPARSER_H