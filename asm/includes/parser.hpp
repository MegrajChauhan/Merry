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

    public:
        Parser() = default;

        bool parse(std::string fname);

        bool new_file();
    };
};

#include "compunit.hpp"

#endif