#ifndef _NODES_
#define _NODES_

#include <memory>
#include <string>
#include "tokens.hpp"

namespace masm
{
    enum NodeKind
    {
        _INC_FILE,
        _VAR_DECLR,
    };

    enum DataType
    {
        BYTE,
        WORD,
        DWORD,
        QWORD,
        ARRAY,
        FLOAT,
        LFLOAT
    };

    struct Base
    {
    };

    struct NodeIncFile : public Base
    {
        size_t ind;
    };

    struct NodeVarDeclr : public Base
    {
        DataType type;
        std::vector<Token> expr;
        std::string name;
        bool is_const = false;
    };

    struct Node
    {
        std::unique_ptr<Base> node;
        NodeKind kind;
        size_t st_line, ed_line;
        size_t st_col, ed_col;
    };
};

#endif