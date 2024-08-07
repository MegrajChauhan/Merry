#ifndef _PARSER_
#define _PARSER_

#include "lexer.hpp"
#include "nodes.hpp"
#include <unordered_map>
#include <vector>

namespace masm
{
    class Parser
    {
        Lexer l;
        std::vector<Node> nodes;
        Token t;
        std::string path;
        bool read_again = true;

    public:
        Parser() = default;

        bool parse(std::string fname);

        bool new_file();

        bool variable_declaration();

        DataType get_datatype(TokenType t);
    };
};

#include "compunit.hpp"

#endif