#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <fstream>
#include <fcntl.h>

#include <TokenStream.h>
#include <IConfigParser.h>

#define DEFAULT_WORKER_CONNECTIONS 512;
#define DEFAULT_USE "epoll";

class ConfigParser : public IConfigParser {
  private:
    Context _ast;
    EventsConfig _eventsConfig;
    std::vector<ServerConfig> _serversConfig;
    
    LocationConfig _parseLocationContext(const Context& locationContext);
    void _processServerDirectives(const Context& context, ServerConfig& config);

    void _parseListen(const Directive& directive, ServerConfig& config);
    void _parseServerNames(const Directive& directive, ServerConfig& config);
    void _parseRoot(const Directive& directive, CommonConfig& config);
    void _parseClientMaxBodySize(const Directive& directive, CommonConfig& config);
    void _parseAllowMethods(const Directive& directive, CommonConfig& config);
    void _parseIndex(const Directive& directive, CommonConfig& config);
    
    void _parseServerContext(const Context& serverContext);
    void _parseEventsContext(const Context& eventsContext);
    void _validateServerContext(const Context& context);
    void _makeAst(const std::string& filename);
    void _makeConfig();
    
  public:
    ConfigParser(const std::string& filename);
    ~ConfigParser();
    Context getAst();
    EventsConfig getEventsConfig();
    std::vector<ServerConfig> getServersConfig();
};

#endif // CONFIGPARSER_H