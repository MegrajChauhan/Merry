#ifndef _LEXER_
#define _LEXER_

/*
 The Lexer will read the contents of the input file and store it.
 It will then lex per request only.
 This way we can be quicker.
*/

#include <fstream> // file IO
#include <string>
#include <unordered_map>
#include <filesystem> // for checking if directory
#include <optional>
// #include "fpos.hpp"

#define should_skip(ch) (ch == ' ' || ch == '\t' || ch == '\r') // add more as needed
#define is_num(ch) (ch >= '0' && ch <= '9')
#define is_alpha(ch) ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
#define is_oper(ch) (ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ':' || ch == ';')

namespace MLang
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

        // some general token types
        _TT_IDENTIFIER, // function names and variable names
        _TT_INT,        // integer numbers like "123", "456" and not the floating points
        _TT_FLOAT,      // floating point numbers

        _TT_EOF,
        _TT_ERR, // error token type
    };

    // the global map to identify the token types
    std::unordered_map<std::string, TokenType> _iden_map_ = {
        {"int", _TT_KEYWORD_INT}, {"float", _TT_KEYWORD_FLOAT}, {"(", _TT_OPERATOR_OPAREN}, {")", _TT_OPERATOR_CPAREN}, {"{", _TT_OPERATOR_OCURLY}, {"}", _TT_OPERATOR_CCURLY}, {":", _TT_OPERATOR_COLON}, {";", _TT_OPERATOR_SEMICOLON}};

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
        // FPos pos; // keep track of pos
        std::string filename;
        std::string filecontents;
        std::string::iterator iter;
        std::string::iterator enditer;
        LexErr error; // any error encountered

        bool check_file_ext(std::string file_name)
        {
            return file_name.ends_with("me");
        }

        // skip all whitespaces, tabs, newlines etc
        void skip_all_unnecessary()
        {
            while (should_skip(*this->iter) && this->iter != this->enditer)
            {
                // this->pos.update_pos(*this->iter);
                this->iter++; // update the iterator
            }
        }

        void skip_whitespaces()
        {
            // just whitespaces
            while (*this->iter == ' ' && this->iter != this->enditer)
            {
                // this->pos.update_pos(*this->iter);
                this->iter++; // update the iterator
            }
        }

        char peek()
        {
            return *(this->iter + 1);
        }

        void consume()
        {
            // this->pos.update_pos(*this->iter);
            this->iter++;
        }

        Token get_num()
        {
            // read a number from the file
            std::string num;
            bool is_float = false;
            while (is_num(*this->iter) || *this->iter == '.')
            {
                if (*this->iter == '.' && is_float == true)
                {
                    std::cerr << "Invalid number: " << num << "\n";
                    abort();
                }
                num.push_back(*this->iter);
                this->consume();
            }
            return Token(is_float ? _TT_FLOAT : _TT_INT, num);
        }

        Token get_iden_or_keyword()
        {
            std::string iden_or_keyword;
            bool has_underscore = false;
            while (is_alpha(*this->iter) || *this->iter == '_')
            {
                if (*this->iter == '_')
                    has_underscore = true;
                iden_or_keyword.push_back(*this->iter);
                this->consume();
            }
            // now check if it is an identifier or a keyword
            // we will save time by seeing if the token has '_'
            // since we don't have keywords that use '_', it must be an identifier
            if (has_underscore)
            {
                return Token(_TT_IDENTIFIER, iden_or_keyword);
            }
            auto _keytype = _iden_map_.find(iden_or_keyword);
            if (_keytype == _iden_map_.end())
            {
                // std::cerr << "Invalid keyword"
            }
        }

    public:
        Lexer(std::string);

        bool open_file_for_lexing();

        Token lex();

        LexErr get_error();
    };

};

#endif