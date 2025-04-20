#include <ConfigParser.h>

ConfigParser::ConfigParser() {};
ConfigParser::~ConfigParser() {};
IConfigParser::~IConfigParser() {};

Context parseBlock(TokenStream& ts) {
    Context ctx;
    // 1) read the block name
    ctx.name = ts.next().value;          // must be IDENTIFIER

    // 2) consume the “{”
    ts.expect(PUNCT, "{");

    // 3) loop until we hit the matching “}”
    while (!ts.accept(PUNCT, "}")) {
        // nested block? IDENT followed by "{"
        if (ts.peek().type == IDENTIFIER
         && ts.peek(1).type == PUNCT
         && ts.peek(1).value == "{")
        {
            // recurse to parse child block
            Context child = parseBlock(ts);
            ctx.children.push_back(child);
        }
        else {
            // directive: IDENT args* ;
            Directive dir;
            dir.name = ts.next().value;   // IDENTIFIER
            // collect zero or more args (IDENT, NUMBER, or STRING)
            while (!ts.accept(PUNCT, ";")) {
                const Token& a = ts.next();
                dir.args.push_back(a.value);
            }
            ctx.directives.push_back(dir);
        }
    }

    return ctx;
}

void ConfigParser::makeAst(TokenStream& ts) {
    _ast.name = "root";

    // While there are tokens before EOF
    while (ts.peek().type != ENDOFFILE) {
        // skip stray semicolons
        if (ts.accept(PUNCT, ";"))
            continue;
        // expect a top‑level block
        if (ts.peek().type == IDENTIFIER && ts.peek(1).value == "{") {
            Context block = parseBlock(ts);
            _ast.children.push_back(block);
        }
        // else {
        //     const Token& t = ts.peek();
        //     throw std::runtime_error(
        //         "Unexpected token at " +
        //         toString(t._line) + ":" + toString(t._col) +
        //         " — expected block name"
        //     );
        // }
        return;
    }
}

Context ConfigParser::getAst() {
    return _ast;
}

ServerConfig ConfigParser::getServerConfig(const std::string& filename) {
    // READ AND EXTRACT FILE
    std::ifstream configFile(filename);
    std::string line;
    while (getline(configFile, line)) {
        _buffer.append(line);
    }
    configFile.close();

    TokenStream ts(_buffer);

    makeAst(ts);


    return _serverConfig;
}


