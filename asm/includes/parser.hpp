#ifndef _PARSER_
#define _PARSER_

#include "lexer.hpp"
#include "nodes.hpp"
#include <unordered_map>
#include <vector>
#include "compunit.hpp"

namespace masm
{
    class Parser
    {
        Lexer l;
        std::vector<Node> nodes;

    public:
        Parser() = default;

        bool parse(std::string fname);
    };
};

#endif