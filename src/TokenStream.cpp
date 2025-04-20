#include <TokenStream.h>

std::string TokenStream::_tokenDesc(TokenType ttype, const std::string& tvalue) const {
    switch (ttype) {
        case IDENTIFIER: return "identifier";
        case NUMBER: return "number";
        case STRING: return "string";
        case PUNCT: return "'" + tvalue + "'";
        case ENDOFFILE: return "end-of-file";
        case COMMENTS: return "comment";
    }
    return "";
}

void TokenStream::_tokenize() {
    while (_pos < _text.size()) {
        char c = _text[_pos];
        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\r') {
            _advance();
            continue;
        }
        // Newline
        if (c == '\n') {
            _advance();
            _line++; _col = 1;
            continue;
        }
        // Comments (# ... to end of line)
        if (c == '#') {
            while (_pos < _text.size() && _text[_pos] != '\n')
                _advance();
            continue;
        }
        // Identifier or keyword: [A-Za-z_][A-Za-z0-9_]* 
        if (isalpha(c) || c == '_' || c == '/') {
            int startCol = _col;
            std::string val;
            while (_pos < _text.size() && (isalnum(_text[_pos]) || _text[_pos] == '_' || _text[_pos] == '/')) {
                val += _text[_pos];
                _advance();
            }
            _tokens.push_back(Token{IDENTIFIER, val, _line, startCol});
            continue;
        }
        // Number: [0-9]+
        if (isdigit(c)) {
            int startCol = _col;
            std::string val;
            while (_pos < _text.size() && isdigit(_text[_pos])) {
                val += _text[_pos];
                _advance();
            }
            _tokens.push_back(Token{NUMBER, val, _line, startCol});
            continue;
        }
        // String literal: " ... "
        if (c == '"') {
            int startCol = _col;
            _advance();  // consume "
            std::string val;
            while (_pos < _text.size() && _text[_pos] != '"') {
                // Allow escaped quotes? (optional)
                val += _text[_pos];
                _advance();
            }
            _expectChar('"');
            _tokens.push_back(Token{STRING, val, _line, startCol});
            continue;
        }
        // Punctuation: { } ; 
        if (c == '{' || c == '}' || c == ';') {
            int startCol = _col;
            std::string val(1, c);
            _advance();
            _tokens.push_back(Token{PUNCT, val, _line, startCol});
            continue;
        }
        // Anything else is an error
        throw std::runtime_error(
            "Unexpected character `" + std::string(1,c)
            + "` at line " + toString(_line)
            + " col " + toString(_col));
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
    _tokens.back().type = ENDOFFILE;
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

bool TokenStream::hasMore() const {
    return peek().type != ENDOFFILE;
}

bool TokenStream::accept(TokenType ttype, const std::string& tval) {
    const Token& token = peek();
    if (token.type == ttype && (tval.empty() || token.value == tval)) {
        next();
        return true;
    }
    return false;
}

void TokenStream::expect(TokenType ttype, const std::string& tval) {
    const Token& token = peek();
    if (token.type != ttype || (!tval.empty() && token.value != tval)) {
        std::string msg = "Parse error at line " + toString(token.line)
                            + " col " + toString(token.column)
                            + ": expected ";
        msg += _tokenDesc(ttype, tval) + ", got `" + token.value + "`";
        throw std::runtime_error(msg);
    }
    next();
}

std::string toString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}
