#ifndef _LEXER_
#define _LEXER_

#include <vector>
#include <string>
#include <unordered_map>
#include <cctype>
#include "error.hpp"
#include "reader.hpp"

namespace mlang
{
    enum TokenType
    {
        // we have different kinds of tokens
        // The keyword tokens
        _TT_KEYWORD_INT,   // "int" keyword
        _TT_KEYWORD_FLOAT, // "float" keyword

        // The operators
        _TT_OPERATOR_OPAREN,    // the '(' operator
        _TT_OPERATOR_CPAREN,    // the ')' operator
        _TT_OPERATOR_OCURLY,    // the '{' operator
        _TT_OPERATOR_CCURLY,    // the '}' operator
        _TT_OPERATOR_COLON,     // the ':' operator
        _TT_OPERATOR_SEMICOLON, // the ';' operator
        _TT_OPERATOR_EQUAL,     // the "=" operator

        // some general token types
        _TT_IDENTIFIER, // function names and variable names
        _TT_INT,        // integer numbers like "123", "456" and not the floating points
        _TT_FLOAT,      // floating point numbers

        _TT_EOF,
        _TT_ERR, // error token type
    };

    // the global map to identify the token types
    static std::unordered_map<std::string, TokenType>
        _iden_map_ =
            {
                {"int", _TT_KEYWORD_INT}, {"float", _TT_KEYWORD_FLOAT}, {"(", _TT_OPERATOR_OPAREN}, {")", _TT_OPERATOR_CPAREN}, {"{", _TT_OPERATOR_OCURLY}, {"}", _TT_OPERATOR_CCURLY}, {":", _TT_OPERATOR_COLON}, {";", _TT_OPERATOR_SEMICOLON}, {"=", _TT_OPERATOR_EQUAL}};

    struct Token
    {
        TokenType type;    // the token's type
        std::string value; // for eg the token _TT_INT could have a value "123" from the code

        Token() = default;

        Token(TokenType, std::string);
    };

    class Lexer
    {
    private:
        Reader reader;
        size_t col_no;
        std::string current_line;
        std::string::iterator curr_char;
        bool eof;

        void consume()
        {
            // this should consume the current character
            // if the lexer has completely consumed the current line, it also needs to advance
            if (curr_char == current_line.end())
            {
                if ((current_line = reader.next_line()) == "EOF")
                {
                    eof = true;
                    return;
                }
                col_no = 0;
                curr_char = current_line.begin();
            }
            else
            {
                col_no++;
                curr_char++;
            }
        }

        void clear_unnecessary()
        {
            // clear all unnecessary characters
            while (std::isspace(static_cast<unsigned char>(*this->curr_char)))
            {
                consume();
            }
        }

        char peek()
        {
            if (curr_char != current_line.end())
                return *(curr_char + 1);
            return '\0';
        }

    public:
        Lexer() = default;

        bool setup_reader(std::string);

        Token lex();
    };
};

#endif