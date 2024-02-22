#ifndef _LEXER_
#define _LEXER_

#include <vector>
#include <string>
#include <unordered_map>
#include <cctype>
#include "reader.hpp"
#include "../utils/colors.hpp"

#define should_skip(ch) (ch == ',' || ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') // add more as needed
#define is_num(ch) (ch >= '0' && ch <= '9')
#define is_alpha(ch) ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
#define is_oper(ch) (ch == ':')

namespace masm
{
    namespace lexer
    {
        enum TokenType
        {
            _TT_ERR, // return for functions
            _TT_EOF,
            _TT_SECTION_DATA,
            _TT_SECTION_TEXT,
            _TT_KEY_STRING, // the ".string" keyword
            _TT_KEY_DB,     // "db" or define byte
            _TT_KEY_PROC,

            _TT_TYPE_STRING,
            _TT_INT,
            _TT_FLOAT,

            _TT_OPER_COLON,

            _TT_IDENTIFIER,

            // for the lexer, even the register names like Ma, Mb are IDENTIFIER, it's interpretation is upto the parser
            _TT_INST_MOV,
            _TT_INST_HLT,

            // we ignore commas, they are not absolutely necessary and the assembler won't even complain
            // about not using it. It is just their to provide readability
        };

        // the global map to identify the token types
        static std::unordered_map<std::string, TokenType>
            _iden_map_ =
                {
                    {"data", _TT_SECTION_DATA}, {"text", _TT_SECTION_TEXT}, {"string", _TT_KEY_STRING}, {"db", _TT_KEY_DB}, {"proc", _TT_KEY_PROC}, {":", _TT_OPER_COLON}, {"mov", _TT_INST_MOV}, {"hlt", _TT_INST_HLT}};

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
            size_t col_no = 0;
            size_t line_num = 0;
            std::string file_contents;
            std::string::iterator curr_char;
            std::string::iterator end;
            std::string filename;

            void consume()
            {
                // this should consume the current character
                // if the lexer has completely consumed the current line, it also needs to advance
                if (curr_char == end)
                {
                    return;
                }
                else
                {
                    if (*curr_char == '\n')
                    {
                        col_no = 0;
                        line_num++;
                        curr_char++;
                        return;
                    }
                    col_no++;
                    curr_char++;
                }
            }

            void consume_comment()
            {
                if (curr_char == end)
                {
                    return;
                }
                col_no = 0;
                while (*curr_char != '\n' && curr_char != end)
                {
                    curr_char++;
                }
                if (curr_char == file_contents.end())
                    return;
                curr_char++;
                line_num++;
                clear_unnecessary();
            }

            void clear_unnecessary()
            {
                // clear all unnecessary characters
                while ((curr_char != end) && (std::isspace(static_cast<int>(*this->curr_char)) || *curr_char == ','))
                {
                    consume();
                }
            }

            char peek()
            {
                if (curr_char != end)
                    return *(curr_char + 1);
                return '\0';
            }

            Token get_iden_or_keyword()
            {
                Token token;
                std::string iden_or_keyword;
                while (is_alpha(*curr_char) || *curr_char == '_')
                {
                    iden_or_keyword.push_back(*curr_char);
                    consume();
                }
                auto kind = _iden_map_.find(iden_or_keyword);
                if (kind == _iden_map_.end())
                {
                    // since the characters we read are valid then it must be an identifier
                    token.type = _TT_IDENTIFIER;
                    token.value = iden_or_keyword;
                }
                else
                {
                    // then it is a keyword
                    token.type = (*kind).second;
                }
                return token;
            }

            Token get_number()
            {
                Token token;
                size_t underscore_count = 0;
                std::string val;
                while (is_num(*curr_char) || *curr_char == '.')
                {
                    if (*curr_char == '.')
                    {
                        underscore_count++;
                        if (underscore_count > 1)
                        {
                            invalid_token(val); // exit
                        }
                    }
                    val.push_back(*curr_char);
                    consume();
                }
                return Token(underscore_count == 1 ? _TT_FLOAT : _TT_INT, val);
            }

            std::string get_current_line()
            {
                std::string line;
                std::fstream temp(filename, std::ios::in); // temporary[Should not fail]
                for (size_t i = 0; i <= line_num; i++)
                {
                    std::getline(temp, line);
                }
                temp.close();
                return line;
            }

        public:
            Lexer() = default;

            bool setup_reader(std::string);

            Token lex();

            std::vector<Token> lex_all();

            void invalid_token(std::string);
            void invalid_token();
        };
    };
};

#endif