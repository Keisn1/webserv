#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H

#include <string>
#include <vector>
#include <cctype>
#include <sstream>
#include <stdexcept>

enum TokenType {
  IDENTIFIER,
  NUMBER,
  STRING,
  PUNCT,
  ENDOFFILE,
  COMMENTS
};

typedef struct Token {
  TokenType type;
  std::string value;
  int line, column;
} Token;

class TokenStream {
  private:
    std::string _text;
    std::vector<Token> _tokens;
    size_t _pos, _index;
    int _line, _col;
    std::string _tokenDesc(TokenType ttype, const std::string& tv) const;
    void _tokenize();
    void _advance();
    void _expectChar(char expected);

    public:
    TokenStream(const std::string& input);
    const Token& peek(int ahead = 0) const;
    Token next();
    void unget();
    bool hasMore() const;
    void expect(TokenType ttype, const std::string& tvalue = "");
    bool accept(TokenType ttype, const std::string& tvalue = "");
};

std::string toString(int value);

#endif // TOKENSTREAM_H