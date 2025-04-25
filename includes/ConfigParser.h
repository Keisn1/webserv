#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <fstream>
#include <fcntl.h>

#include <TokenStream.h>
#include <IConfigParser.h>

class ConfigParser : public IConfigParser {
  private:
    std::string _filename;
    Context _ast;
    EventsConfig _eventsConfig;
    std::vector<ServerConfig> _serversConfig;
    void _parseDirectiveOrBlock(TokenStream& tokenStream, Context& currentBlock);
    LocationConfig _parseLocationContext(const Context& locationContext);
    void _processServerDirectives(const Context& context, ServerConfig& config);
    void _parseServerContext(const Context& serverContext);
    void _parseEventsContext(const Context& eventsContext);
    bool _findDirective(const Context& context, const std::string& identifierKey);
    void _validateServerContext(const Context& context);
    void _validateFilename();
    void _parseWorkerConnections(const Directive& directive, EventsConfig& config);
    void _parseUse(const Directive& directive, EventsConfig& config);
    void _parseListen(const Directive& directive, ServerConfig& config);
    void _parseServerNames(const Directive& directive, ServerConfig& config);
    void _parseCgiExt(const Directive& cgiExt, ServerConfig& config);
    void _parseRoot(const Directive& directive, CommonConfig& config);
    void _parseClientMaxBodySize(const Directive& directive, CommonConfig& config);
    void _parseAllowMethods(const Directive& directive, CommonConfig& config);
    void _parseIndex(const Directive& directive, CommonConfig& config);
    void _parseErrorPage(const Directive& directive, CommonConfig& config);
    void _parseAutoindex(const Directive& directive, CommonConfig& config);
    void _makeAst();
    void _makeConfig();
    
  public:
    ConfigParser(const std::string& filename);
    ~ConfigParser();
    Context getAst();
    EventsConfig getEventsConfig();
    std::vector<ServerConfig> getServersConfig();
};

#endif // CONFIGPARSER_H