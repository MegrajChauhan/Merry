#ifndef _LEXER_
#define _LEXER_

/*
 The Lexer will read the contents of the input file and store it.
 It will then lex per request only.
 This way we can be quicker.
*/

#include <fstream> // file IO
#include <string>
#include <filesystem> // for checking if directory
#include "fpos.hpp"

#define should_skip(ch) (ch == ' ' || ch == '\t' || ch == '\r') // add more as needed

namespace MLang
{
    enum TokenType
    {
        // we have different kinds of tokens
        // The keyword tokens
        _TT_KEYWORD_INT, // "int" keyword

        // The operators
        _TT_OPERATOR_OPAREN,    // the '(' operator
        _TT_OPERATOR_CPAREN,    // the ')' operator
        _TT_OPERATOR_OCURLY,    // the '{' operator
        _TT_OPERATOR_CCURLY,    // the '}' operator
        _TT_OPERATOR_COLON,     // the ':' operator
        _TT_OPERATOR_SEMICOLON, // the ';' operator

        // some general token types
        _TT_IDENTIFIER, // function names and variable names
        _TT_INT,        // integer numbers like "123", "456" and not the floating points

        _TT_EOF,
        _TT_ERR, // error token type
    };

    struct Token
    {
        TokenType type;    // the token's type
        std::string value; // for eg the token _TT_INT could have a value "123" from the code

        Token(TokenType, std::string);

    };

    enum LexErr
    {
        IS_DIR,
        DOES_NOT_EXIST,
        FAILED_TO_OPEN,
        FILE_EMPTY
    };

    class Lexer
    {
    private:
        FPos pos;          // keep track of pos
        std::string filename;
        std::string filecontents;
        std::string::iterator iter;
        LexErr error; // any error encountered

        bool check_file_ext(std::string file_name)
        {
            return file_name.ends_with("me");
        }

        // skip all whitespaces, tabs, newlines etc
        void skip_all_unnecessary()
        {
            // char temp = this->file.get();
            // while (should_skip(temp) && temp != EOF)
            // {
            //     this->pos.update_pos(temp);
            //     temp = this->file.get();
            // }
        }

        void skip_whitespaces()
        {
            // just whitespaces
            // char temp = this->file.get();
            // while (temp == ' ' && temp != EOF)
            // {
            //     this->pos.update_pos(temp);
            //     temp = this->file.get();
            // }
        }

    public:
        Lexer(std::string);

        bool open_file_for_lexing();

        Token lex();
    };

};

#endif