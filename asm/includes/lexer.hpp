#ifndef _LEXER_
#define _LEXER_

#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "tokens.hpp"
#include "error.hpp"

namespace masm
{
    class Lexer
    {
        std::string path;
        std::string fileconts;
        std::string::iterator iter;
        size_t offset = 0, line = 0, col = 0;

    public:
        Lexer() = default;

        Lexer(std::string filename);

        bool validate_file();

        bool validate_file(std::string);

        bool init_lexer();

        Token next_token();

        std::string extract_current_line();

        std::string extract_just_text();

        size_t get_line();
    };
};

#endif