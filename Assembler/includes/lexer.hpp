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
            _TT_KEY_DW,
            _TT_KEY_DD,
            _TT_KEY_DQ,
            _TT_KEY_PROC,

            _TT_STRING,
            _TT_INT,
            _TT_FLOAT,

            _TT_OPER_COLON,

            _TT_IDENTIFIER,

            // for the lexer, even the register names like Ma, Mb are IDENTIFIER, it's interpretation is upto the parser
            _TT_INST_NOP,
            _TT_INST_MOV,
            _TT_INST_HLT,

            _TT_INST_MOVQ,

            _TT_INST_MOVB,
            _TT_INST_MOVW,
            _TT_INST_MOVD,

            _TT_INST_MOVEB,
            _TT_INST_MOVEW,
            _TT_INST_MOVED,

            _TT_INST_MOVSXB,
            _TT_INST_MOVSXW,
            _TT_INST_MOVSXD,

            _TT_INST_OUTR,
            _TT_INST_UOUTR,

            // we ignore commas, they are not absolutely necessary and the assembler won't even complain
            // about not using it. It is just their to provide readability
        };

        // the global map to identify the token types
        static std::unordered_map<std::string, TokenType> _iden_map_ =
            {
                {"data", _TT_SECTION_DATA},
                {"text", _TT_SECTION_TEXT},
                {"string", _TT_KEY_STRING},
                {"db", _TT_KEY_DB},
                {"dw", _TT_KEY_DW},
                {"dd", _TT_KEY_DD},
                {"dq", _TT_KEY_DQ},
                {"proc", _TT_KEY_PROC},
                {":", _TT_OPER_COLON},
                {"mov", _TT_INST_MOV},
                {"hlt", _TT_INST_HLT},
                {"movq", _TT_INST_MOVQ},
                {"movb", _TT_INST_MOVB},
                {"movw", _TT_INST_MOVW},
                {"movd", _TT_INST_MOVD},
                {"moveb", _TT_INST_MOVEB},
                {"movew", _TT_INST_MOVEW},
                {"moved", _TT_INST_MOVED},
                {"movsxb", _TT_INST_MOVSXB},
                {"movsxw", _TT_INST_MOVSXW},
                {"movsxd", _TT_INST_MOVSXD},
                {"nop", _TT_INST_NOP},
                {"outr", _TT_INST_OUTR},
                {"uoutr", _TT_INST_UOUTR},
        };

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
            std::filesystem::path path;
            size_t idx = 0;

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
                        idx++;
                        return;
                    }
                    col_no++;
                    curr_char++;
                    idx++;
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
                    idx++;
                }
                if (curr_char == file_contents.end())
                    return;
                curr_char++;
                line_num++;
                idx++;
                clear_unnecessary();
            }

            void clear_unnecessary()
            {
                // clear all unnecessary characters
                while ((curr_char != end) && should_skip(*curr_char))
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
                while (((*curr_char != ' ' && (is_alpha(*curr_char) || is_num(*curr_char) && !is_oper(*curr_char))) || *curr_char == '_'))
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

            std::string get_string()
            {
                char starting_quote = *curr_char;
                std::string string;
                consume();
                while (*curr_char != starting_quote && curr_char != end)
                {
                    if (*curr_char == '\\')
                    {
                        consume(); // The next character will be added
                        if (curr_char == end)
                        {
                            lex_error("The string was never terminated. Reached EOF");
                        }
                        string += *curr_char;
                        consume();
                    }
                    else
                    {
                        string += *curr_char;
                        consume();
                    }
                }
                if (*curr_char == starting_quote)
                {
                    // the string was indeed terminated
                    // we also need to check if there is another '.'
                    consume();
                    if (*curr_char == '.')
                    {
                        consume();
                        clear_unnecessary();
                        consume_comment();
                        string += get_string(); // a recursive function call to get all the needed strings
                    }
                }
                else
                {
                    lex_error("The string was never terminated. Reached EOF");
                }
                return string;
            }

        public:
            Lexer() = default;

            bool setup_reader(std::string);

            Token lex();

            size_t get_curr_line() { return line_num; }

            auto get_path() { return path; }

            std::vector<Token> lex_all();

            void invalid_token(std::string);
            void invalid_token();

            void lex_error(std::string);

            void parse_err_whole_line(std::string);
            void parse_err_expected_colon(std::string);

            void parse_err_previous_token(std::string, std::string);

            void parse_error(std::string);
        };
    };
};

#endif