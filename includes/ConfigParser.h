#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <fstream>
#include <fcntl.h>

#include <TokenStream.h>
#include <IConfigParser.h>

class ConfigParser : public IConfigParser {
  private:
    std::string _buffer;
    Context _ast;
    ServerConfig _serverConfig;
    void _makeServerConfig();
    
  public:
    ConfigParser();
    ~ConfigParser();
    void makeAst(TokenStream& ts);
    ServerConfig getServerConfig(const std::string& filename);
    Context getAst();
};

#endif // CONFIGPARSER_H