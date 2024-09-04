#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "defs.hpp"
#include "lexer.hpp"

namespace masm
{
    struct Variable
    {
        std::string name;
        std::string value;
        std::vector<Token> expr;
        bool is_expr = false;
        DataType type;
        size_t line;
        std::shared_ptr<std::string> file;
    };

    // changing this might be a good idea
    struct SymbolTable
    {
        std::vector<Variable> variables;
        std::unordered_map<std::string, size_t> _var_list; // for quick checking
        std::unordered_map<std::string, Variable> _const_list;
    };
};

#endif