#ifndef _PARSER_
#define _PARSER_

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "symtable.hpp"
#include "nodes.hpp"

namespace masm
{
    static std::vector<std::string> used_files;

    class Parser
    {
        Lexer _l;
        std::shared_ptr<std::string> filename;
        SymbolTable symtable;
        std::unordered_map<std::string, bool> proc_list;
    };
};

#endif