#include "../includes/lexer.hh"
#include <cctype>
#define COMMENT_SYMBOL ';'

void merry::front_end::Lexer::advance(){
    if (idx < _contents.size()-1)
    {
        idx += 1;
        _c = _contents.at(idx);
        loc.update(_c);
    }
}

void merry::front_end::Lexer::skip_whitespace(){
    while (isskip() && idx < _contents.size() - 1)
    {
        advance();
    }
}

std::vector<merry::front_end::Token> merry::front_end::Lexer::lex_all(){
    std::vector<Token> ret;
    Token next = next_token();
    ret.push_back(next);
    while (next.get_type() != TokenType::TT_EOF)
    {
        next = next_token();
        ret.push_back(next);
    }
    return ret;
}

merry::front_end::Token merry::front_end::Lexer::next_token(){
    skip_whitespace();
    if (idx >= _contents.size() - 1)
        return Token(TokenType::TT_EOF, "\0", loc);
    // first off check for EOF
    while (_c == COMMENT_SYMBOL && idx < _contents.size() - 1)
    {
        while (_c != '\n' && idx < _contents.size() - 1)
            advance();
        skip_whitespace();
    }

    skip_whitespace();

    if (idx >= _contents.size() - 1)
        return Token(TokenType::TT_EOF, "\0", loc);
    switch (_c)
    {
        case ':':
        case ',': {
            char pref = _c;
            advance();
            std::string value;
            value.push_back(pref);
            return Token(static_cast<TokenType>(pref), value, loc);
        } break;
    }
    // Check for any ids / numbers
    if (isalnum(_c) || _c == '_')
    {
        std::string buffer;
        buffer.push_back(_c);
        advance();
        while (isalnum(_c) || _c == '_' || _c == '.')
        {
            buffer.push_back(_c);
            advance();
            if(idx+1 >= _contents.size()){
                buffer.push_back(_c);
                break;
            }
        }
        if (isdigit(buffer.at(0)))
        {
            return Token(TokenType::NUMBER, buffer, loc);
        }
        return Token(TokenType::ID, buffer, loc);
    }
    std::cout << "Invalid character `" << _c << "`\n";
    exit(1);
}

merry::front_end::Token merry::front_end::Lexer::peek(){
    size_t curr_idx = idx;
    Token temp = next_token();
    idx = curr_idx;
    _c = _contents.at(idx);
    return temp;
}