#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace masm
{
    enum DataType
    {
        BYTE,
        WORD,
        DWORD,
        QWORD,
        FLOAT,
        LFLOAT,
        STRING,
        RESB,
        RESW,
        RESD,
        RESQ
    };

    struct Variable
    {
        std::string name;
        std::string value;
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