#include <TokenStream.h>

std::string TokenStream::_tokenDesc(TokenType ttype, const std::string& tvalue) const {
    switch (ttype) {
        case IDENTIFIER: return "identifier";
        case NUMBER: return "number";
        case STRING: return "string";
        case PUNCT: return "'" + tvalue + "'";
        case END_OF_FILE: return "end-of-file";
    }
    return "";
}

void TokenStream::_tokenize() {
    while (_pos < _text.size()) {
        char c = _text[_pos];
        
        if (c == ' ' || c == '\t' || c == '\r') {                           // whitespace
            _advance();
        } else if (c == '\n') {                                             // newline
            _advance();
            _line++; _col = 1;
        } else if (c == '#') {                                              // comments
            while (_pos < _text.size() && _text[_pos] != '\n')
                _advance();
        } else if (isalpha(c) || c == '_' || c == '/' || c == '.') {                    // identifier
            int startCol = _col;
            std::string val;
            while (_pos < _text.size() && (isalnum(_text[_pos]) || _text[_pos] == '_' || _text[_pos] == '/' || _text[_pos] == '.')) {
                val += _text[_pos];
                _advance();
            }
            _tokens.push_back(Token{IDENTIFIER, val, _line, startCol});
        } else if (isdigit(c)) {                                            // number
            int startCol = _col;
            std::string val;
            while (_pos < _text.size() && isdigit(_text[_pos])) {
                val += _text[_pos];
                _advance();
            }
            _tokens.push_back(Token{NUMBER, val, _line, startCol});
        } else if (c == '"') {                                              // string literal
            int startCol = _col;
            _advance();  // consume "
            std::string val;
            while (_pos < _text.size() && _text[_pos] != '"') {
                val += _text[_pos];
                _advance();
            }
            _expectChar('"');
            _tokens.push_back(Token{STRING, val, _line, startCol});
        } else if (c == '{' || c == '}' || c == ';') {                      // punctuation
            int startCol = _col;
            std::string val(1, c);
            _advance();
            _tokens.push_back(Token{PUNCT, val, _line, startCol});
        } else {                                                            // error
            throw std::runtime_error(
                "Unexpected character `" + std::string(1,c)
                + "` at line " + toString(_line)
                + " col " + toString(_col));
        }
    }
}

void TokenStream::_advance() {
    _pos++;
    _col++;
}

void TokenStream::_expectChar(char expected) {
    if (_pos >= _text.size() || _text[_pos] != expected) {
        throw std::runtime_error(
            "Expected '" + std::string(1, expected)
            + "' at line " + toString(_line)
            + " col " + toString(_col));
    }
    _advance();
}

TokenStream::TokenStream(const std::string& input) : _text(input), _pos(0), _line(1), _col(0) {
    _tokenize();
    _tokens.push_back(Token());
    _tokens.back().type = END_OF_FILE;
    _tokens.back().value = "";
    _tokens.back().line  = _line;
    _tokens.back().column   = _col;
    _index = 0;
}

const Token& TokenStream::peek(int ahead) const {
    size_t i = _index + ahead;
    if (i >= _tokens.size()) {
        i = _tokens.size() - 1;
    }
    return _tokens[i];
}

Token TokenStream::next() {
    const Token& token = peek(0);
    if (_index < _tokens.size()) {
        _index++;
    }
    return token;
}

void TokenStream::unget() {
    if (_index > 0) {
        _index--;
    }
}

bool TokenStream::hasMore() const {
    return peek().type != END_OF_FILE;
}

bool TokenStream::accept(TokenType ttype, const std::string& tvalue) {
    const Token& token = peek();
    if (token.type == ttype && (tvalue.empty() || token.value == tvalue)) {
        next();
        return true;
    }
    return false;
}

void TokenStream::expect(TokenType ttype, const std::string& tvalue) {
    const Token& token = peek();
    if (token.type != ttype || (!tvalue.empty() && token.value != tvalue)) {
        std::string msg = "Parse error at line " + toString(token.line)
                            + " col " + toString(token.column)
                            + ": expected ";
        msg += _tokenDesc(ttype, tvalue) + ", got `" + token.value + "`";
        throw std::runtime_error(msg);
    }
    next();
}

std::vector<std::string> TokenStream::collectArguments(const std::set<std::string>& terminators) {
    std::vector<std::string> args;
    while (hasMore()) {
        Token t = peek();
        if (t.type == PUNCT && terminators.count(t.value)) {
            break;
        }
        args.push_back(next().value);
    }
    return args;
}

std::string toString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}
